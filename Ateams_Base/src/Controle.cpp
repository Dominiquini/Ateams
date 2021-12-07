#include "Controle.hpp"

using namespace std;

pthread_mutex_t mutex_pop;	// Mutex que protege a populacao principal
pthread_mutex_t mutex_cont;	// Mutex que protege as variaveis de criacao de novas solucoes
pthread_mutex_t mutex_info;	// Mutex que protege a impressao das informacoes da execucao
pthread_mutex_t mutex_exec;	// Mutex que protege as informacoes de execucao

sem_t semaphore;			// Semaforo que controla o acesso dos algoritmos ao processador


Controle* Controle::getInstance(char* xml)
{
	if(instance == NULL)
	{
		instance = new Controle();
	}
	else
	{
		terminate();

		return getInstance(xml);
	}

	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch)
	{
		char* message = XMLString::transcode(toCatch.getMessage());
		cout << "Error During Initialization!" << endl;
		cout << "Exception Message Is: " << capitalize(message) << endl << endl;
		XMLString::release(&message);

		exit(1);
	}

	SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
	parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
	parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);

	DefaultHandler* defaultHandler = instance;
	parser->setContentHandler(defaultHandler);
	parser->setErrorHandler(defaultHandler);

	try
	{
		parser->parse(xml);
	}
	catch(const char* toCatch)
	{
		cout << endl << toCatch << endl << endl;

		exit(1);
	}
	catch(string& toCatch)
	{
		cout << endl << toCatch << endl << endl;

		exit(1);
	}
	catch(const XMLException& toCatch)
	{
		char* message = XMLString::transcode(toCatch.getMessage());
		cout << endl << "Exception Message Is: " << capitalize(message) << endl << endl;
		XMLString::release(&message);

		exit(1);
	}
	catch(const SAXParseException& toCatch)
	{
		char* message = XMLString::transcode(toCatch.getMessage());
		cout << endl << "Exception Message Is: " << capitalize(message) << endl << endl;
		XMLString::release(&message);

		exit(1);
	}
	catch(...)
	{
		cout << endl << "Unexpected Exception!" << endl << endl;

		exit(1);
	}

	delete parser;
	XMLPlatformUtils::Terminate();

	return instance;
}

void Controle::terminate()
{
	delete instance;
	instance = NULL;
}

void Controle::startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs)
{
	char* element = XMLString::transcode(localname);

	if(strcasecmp(element, "Controller") == 0)
	{
		char* parameter = NULL;
		char* value = NULL;

		for(unsigned int ix = 0; ix < attrs.getLength(); ++ix)
		{
			parameter = XMLString::transcode(attrs.getQName(ix));
			value = XMLString::transcode(attrs.getValue(ix));

			if(!setParameter(parameter, value))
			{
				throw string("Invalid Parameter: \" ").append(parameter).append(" \"");
			}

			XMLString::release(&parameter);
			XMLString::release(&value);
		}
	}
	else if(strcasecmp(element, "Heuristics") == 0)
	{
		algs = new vector<Heuristica*>();
	}
	else
	{
		Heuristica* newHeuristic = NULL;

		if(strcasecmp(element, "SimulatedAnnealing") == 0)
			newHeuristic = new Annealing();

		if(strcasecmp(element, "TabuSearch") == 0)
			newHeuristic = new Tabu();

		if(strcasecmp(element, "GeneticAlgorithm") == 0)
			newHeuristic = new Genetico();

		if(newHeuristic != NULL)
		{
			char* parameter = NULL;
			char* value = NULL;

			for(unsigned int ix = 0; ix < attrs.getLength(); ++ix)
			{
				parameter = XMLString::transcode(attrs.getQName(ix));
				value = XMLString::transcode(attrs.getValue(ix));

				if(!newHeuristic->setParameter(parameter, value))
				{
					throw string("Invalid Parameter: \" ").append(parameter).append(" \"");
				}

				XMLString::release(&parameter);
				XMLString::release(&value);
			}

			addHeuristic(newHeuristic);
		}
	}

	XMLString::release(&element);
}

bool Controle::setParameter(const char* parameter, const char* value)
{
	if(strcasecmp(parameter, "iterAteams") == 0)
	{
		sscanf(value, "%d", &iterAteams);
	}
	else if(strcasecmp(parameter, "tentAteams") == 0)
	{
		sscanf(value, "%d", &tentAteams);
	}
	else if(strcasecmp(parameter, "maxTempoAteams") == 0)
	{
		sscanf(value, "%d", &maxTempo);
	}
	else if(strcasecmp(parameter, "numThreads") == 0)
	{
		sscanf(value, "%d", &numThreads);
	}
	else if(strcasecmp(parameter, "tamPopAteams") == 0)
	{
		sscanf(value, "%d", &tamPop);
	}
	else if(strcasecmp(parameter, "critUnicidade") == 0)
	{
		sscanf(value, "%d", &critUnicidade);
	}
	else if(strcasecmp(parameter, "makespanBest") == 0)
	{
		sscanf(value, "%d", &makespanBest);
	}
	else
	{
		return false;
	}

	return true;
}

void Controle::statusInfoScreen(bool status)
{
	this->activeListener = status;
}

void Controle::commandLineParameters()
{
	int p = -1;

	if((p = findPosArgv(argv, *argc, (char*)"--iterAteams")) != -1)
		setParameter("iterAteams", argv[p]);

	if((p = findPosArgv(argv, *argc, (char*)"--numThreads")) != -1)
		setParameter("numThreads", argv[p]);

	if((p = findPosArgv(argv, *argc, (char*)"--tentAteams")) != -1)
		setParameter("tentAteams", argv[p]);

	if((p = findPosArgv(argv, *argc, (char*)"--maxTempoAteams")) != -1)
		setParameter("maxTempoAteams", argv[p]);

	if((p = findPosArgv(argv, *argc, (char*)"--tamPopAteams")) != -1)
		setParameter("tamPopAteams", argv[p]);

	if((p = findPosArgv(argv, *argc, (char*)"--critUnicidade")) != -1)
		setParameter("critUnicidade", argv[p]);

	if((p = findPosArgv(argv, *argc, (char*)"--makespanBest")) != -1)
		setParameter("makespanBest", argv[p]);
}

Controle::Controle()
{
	this->algs = NULL;
	this->pop = NULL;

	this->tamPop = 500;
	this->critUnicidade = 1;
	this->iterAteams = 250;
	this->tentAteams = 100;
	this->maxTempo = 3600;
	this->numThreads = 4;
	this->makespanBest = -1;
	this->activeListener = false;

	this->time1 = this->time2 = 0;
	this->iterMelhora = 0;
	this->execThreads = 0;

	execAlgs = new list<Heuristica_Listener*>;
	actAlgs = new list<list<Heuristica_Listener*>::iterator >;
	actThreads = 0;

	pthread_mutex_init(&mutex_pop, NULL);
	pthread_mutex_init(&mutex_cont, NULL);
	pthread_mutex_init(&mutex_info, NULL);
	pthread_mutex_init(&mutex_exec, NULL);
}

Controle::~Controle()
{
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;

	for(iter = pop->begin(); iter != pop->end(); iter++)
		delete *iter;

	pop->clear();
	delete pop;

	vector<Heuristica*>::reverse_iterator it;

	cout << endl << endl << "Executions: " << execThreads << endl << endl;
	for(it = algs->rbegin(); it != algs->rend(); it++)
		delete *it;

	algs->clear();
	delete algs;

	for(list<Heuristica_Listener*>::iterator it = execAlgs->begin(); it != execAlgs->end(); it++)
		delete *it;

	execAlgs->clear();
	delete execAlgs;

	actAlgs->clear();
	delete actAlgs;

	pthread_mutex_destroy(&mutex_pop);
	pthread_mutex_destroy(&mutex_cont);
	pthread_mutex_destroy(&mutex_info);
	pthread_mutex_destroy(&mutex_exec);
}

double Controle::sumFitnessMaximize(set<Problema*, bool(*)(Problema*, Problema*)> *probs, int n)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::const_iterator iter;
	double sum = 0, i = 0;

	for(i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++)
		sum += (*iter)->getFitnessMaximize();

	return sum;
}

double Controle::sumFitnessMaximize(vector<Problema*> *probs, int n)
{
	vector<Problema*>::const_iterator iter;
	double sum = 0, i = 0;

	for(i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++)
		sum += (*iter)->getFitnessMaximize();

	return sum;
}

double Controle::sumFitnessMinimize(set<Problema*, bool(*)(Problema*, Problema*)> *probs, int n)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::const_iterator iter;
	double sum = 0, i = 0;

	for(i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++)
		sum += (*iter)->getFitnessMinimize();

	return sum;
}

double Controle::sumFitnessMinimize(vector<Problema*> *probs, int n)
{
	vector<Problema*>::const_iterator iter;
	double sum = 0, i = 0;

	for(i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++)
		sum += (*iter)->getFitnessMinimize();

	return sum;
}

set<Problema*, bool(*)(Problema*, Problema*)>::iterator Controle::selectRouletteWheel(set<Problema*, bool(*)(Problema*, Problema*)>* probs, double fitTotal)
{
	// Armazena o fitness total da populacao
	unsigned int sum = (unsigned int)fitTotal;
	// Um numero entre zero e "sum" e sorteado
	unsigned int randWheel = xRand(0, sum+1);

	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	for(iter = probs->begin(); iter != probs->end(); iter++)
	{
		sum -= (int)(*iter)->getFitnessMaximize();
		if(sum <= randWheel)
		{
			return iter;
		}
	}
	return (probs->begin());
}

vector<Problema*>::iterator Controle::selectRouletteWheel(vector<Problema*>* probs, double fitTotal)
{
	// Armazena o fitness total da populacao
	unsigned int sum = (unsigned int)fitTotal;
	// Um numero entre zero e "sum" e sorteado
	unsigned int randWheel = xRand(0, sum+1);

	vector<Problema*>::iterator iter;
	for(iter = probs->begin(); iter != probs->end(); iter++)
	{
		sum -= (int)(*iter)->getFitnessMaximize();
		if(sum <= randWheel)
		{
			return iter;
		}
	}
	return probs->begin();
}

Heuristica* Controle::selectRouletteWheel(vector<Heuristica*>* heuristic, unsigned int probTotal)
{
	if(heuristic == NULL || heuristic->size() == 0)
	{
		cout << "No Heuristics Defined!" << endl << endl;

		exit(1);
	}

	// Armazena o fitness total da populacao
	unsigned int sum = probTotal;
	// Um numero entre zero e "sum" e sorteado
	unsigned int randWheel = xRand(0, sum+1);

	for(int i = 0; i < (int)heuristic->size(); i++)
	{
		sum -= heuristic->at(i)->prob;
		if(sum <= randWheel)
		{
			return heuristic->at(i);
		}
	}
	return heuristic->at(0);
}

vector<Problema*>::iterator Controle::selectRandom(vector<Problema*>* probs)
{
	unsigned int randWheel = xRand(0, probs->size());

	vector<Problema*>::iterator iter = probs->begin();
	for(unsigned long i = 0; iter != probs->end() && i < randWheel; iter++, i++);

	return iter;
}

list<Problema*>::iterator Controle::findSol(list<Problema*> *vect, Problema *p)
{
	list<Problema*>::iterator iter;

	for(iter = vect->begin(); iter != vect->end(); iter++)
		if(fnequal1((*iter), p))
			return iter;

	return iter;
}


void Controle::addHeuristic(Heuristica* alg)
{
	algs->push_back(alg);

	sort(algs->begin(), algs->end(), cmpAlg);
}

list<Problema*>* Controle::getPop()
{
	list<Problema*>* sol = new list<Problema*>();
	set<Problema*, bool(*)(Problema*, Problema*)>::const_iterator iter;

	for(iter = pop->begin(); iter != pop->end(); iter++)
		sol->push_back(*iter);

	return sol;
}

Problema* Controle::getSol(int n)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::const_iterator iter = pop->begin();

	for(int i = 0; i <= n && iter != pop->end(); iter++);

	return Problema::copySoluction(**(--iter));
}

void Controle::getInfo(ExecInfo *info)
{
	info->diffTime = difftime(time2, time1);
	info->numExecs = execThreads;

	info->worstFitness = Problema::worst;
	info->bestFitness = Problema::best;
	info->expSol = Problema::totalNumInst;
}

Problema* Controle::start(list<Problema*>* popInicial)
{
	bool(*fn_pt)(Problema*, Problema*) = critUnicidade == 1 ? fncomp1 : fncomp2;
	pop = new set<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	sem_init(&semaphore, 0, numThreads);

	time(&time1);

	pthread_t *threads = (pthread_t*)malloc(iterAteams * sizeof(pthread_t));
	pthread_t threadTime;
	void* temp = NULL;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	geraPop(popInicial);

	if(pop->size() == 0)
	{
		cout << endl << "No Initial Solution Found!" << endl << endl;

		exit(1);
	}

	iterMelhora = 0;

	Problema::best = (*pop->begin())->getFitness();
	Problema::worst = (*pop->rbegin())->getFitness();

	cout << endl << "Worst Initial Solution: " << Problema::worst << endl << endl;
	cout << "Best Initial Solution: " << Problema::best << endl << endl << endl;

	pair<int, Controle*>* par = NULL;
	long int ins = 0;
	execThreads = 0;

	if(activeListener)
	{
		pthread_t threadAnimation;

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		pthread_create(&threadAnimation, &attr, pthrAnimation, NULL);
	}

	if(pthread_create(&threadTime, &attr, pthrTime, (void*)this) != 0)
	{
		cout << endl << endl << "Thread Creation Error! (pthrTime)" << endl << endl;
		exit(1);
	}

	for(int execAteams = 0; execAteams < iterAteams; execAteams++)
	{
		par = new pair<int, Controle*>();
		par->first = execAteams+1;
		par->second = this;

		if(pthread_create(&threads[execAteams], &attr, pthrExec, (void*)par) != 0)
		{
			cout << endl << endl << "Thread Creation Error! (pthrExec)" << endl << endl;
			exit(1);
		}
	}

	for(int execAteams = 0; execAteams < iterAteams; execAteams++)
	{
		pthread_join(threads[execAteams], &temp);
		ins += (uintptr_t)temp;
	}

	PARAR = true;

	pthread_join(threadTime, NULL);

	cout << endl << "Swapped Solutions: " << ins << endl;

	free(threads);

	pthread_attr_destroy(&attr);

	time(&time2);

	if(activeListener)
		glutDestroyWindow(window);

	return Problema::copySoluction(**(pop->begin()));
}

inline int Controle::exec(int idThread)
{
	list<Heuristica_Listener*>::iterator listener_iterator;
	vector<Problema*> *newSoluctions = NULL;
	Heuristica_Listener* listener = NULL;
	Heuristica* alg = NULL;
	pair<int, int>* insert;
	string execNames;
	int contrib = 0;

	pthread_mutex_lock(&mutex_info);
	{
		alg = selectRouletteWheel(algs, Heuristica::numHeuristic);
		listener = new Heuristica_Listener(alg, idThread);

		actThreads++;

		listener_iterator = execAlgs->insert(execAlgs->begin(), listener);
		actAlgs->push_back(listener_iterator);

		execNames = "";
		for(list<list<Heuristica_Listener*>::iterator >::iterator it = actAlgs->begin(); it != actAlgs->end(); it++)
			execNames = execNames + (**it)->info + " ";

		printf(">>> ALG: %s | ..................................................... | QUEUE: (%d : %s)\n", listener->info.c_str(), actThreads, execNames.c_str());
	}
	pthread_mutex_unlock(&mutex_info);

	if(this->activeListener)
		newSoluctions = alg->start(pop, listener);
	else
		newSoluctions = alg->start(pop, NULL);

	pthread_mutex_lock(&mutex_pop);
	{
		double oldBest = Problema::best;

		insert = addSol(newSoluctions);
		execThreads++;

		double newBest = Problema::best;

		if(Problema::melhora(oldBest, newBest) > 0)
			iterMelhora = 0;
		else
			iterMelhora++;
	}
	pthread_mutex_unlock(&mutex_pop);

	newSoluctions->clear();
	delete newSoluctions;

	pthread_mutex_lock(&mutex_info);
	{
		actThreads--;

		list<list<Heuristica_Listener*>::iterator >::iterator exec = find(actAlgs->begin(), actAlgs->end(), listener_iterator);

		actAlgs->erase(exec);

		printf("<<< ALG: %s | ITER: %.3d | FITNESS: %.6d:%.6d | CONTRIB: %.3d:%.3d", listener->info.c_str(), execThreads, (int)Problema::best, (int)Problema::worst, insert->first, insert->second);

		execNames = "";
		for(list<list<Heuristica_Listener*>::iterator >::iterator it = actAlgs->begin(); it != actAlgs->end(); it++)
			execNames = execNames + (**it)->info + " ";

		printf(" | QUEUE: (%d : %s)\n", actThreads, execNames.c_str());
	}
	pthread_mutex_unlock(&mutex_info);

	contrib = insert->second;

	delete insert;

	return contrib;
}

void* Controle::pthrExec(void *obj)
{
	pair<int, Controle*>* in = (pair<int, Controle*>*)obj;
	int execAteams = in->first;
	Controle *ctr = in->second;
	intptr_t ins = 0;

	delete in;

	sem_wait(&semaphore);

	if(PARAR != true)
	{
		ins = ctr->exec(execAteams);

		if(ctr->iterMelhora > ctr->tentAteams || (ctr->makespanBest != -1 && Problema::melhora(ctr->makespanBest, Problema::best) >= 0))
			PARAR = true;
	}

	sem_post(&semaphore);

	return (void*)ins; 		//	pthread_exit((void*)ins);
}

void* Controle::pthrTime(void *obj)
{
	Controle *ctr = (Controle*)obj;
	time_t rawtime;

	while(PARAR == false)
	{
		time(&rawtime);

		if((int)difftime(rawtime, ctr->time1) > ctr->maxTempo)
			PARAR = true;

		sleep(1);
	}

	return NULL;			//	pthread_exit(NULL);
}

void* Controle::pthrAnimation(void* in)
{
	/* Cria a tela */
	glutInit(Controle::argc, Controle::argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(1250, 500);
	glutInitWindowPosition(0, 0);
	window = glutCreateWindow(Controle::argv[0]);

	/* Define as funcoes de desenho */
	glutDisplayFunc(Controle::display);
	glutIdleFunc(Controle::display);
	glutReshapeFunc(Controle::reshape);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0);

	/* Loop principal do programa */
	glutMainLoop();

	return NULL;			//	pthread_exit(NULL);
}

inline pair<int, int>* Controle::addSol(vector<Problema*> *news)
{
	pair<set<Problema*, bool(*)(Problema*, Problema*)>::iterator, bool> ret;
	vector<Problema*>::const_iterator iterNews;
	int nins = 0, nret = news->size();
	Problema* pointSol = NULL;

	for(iterNews = news->begin(); iterNews != news->end(); iterNews++)
	{
		if(Problema::melhora(**pop->rbegin(), **iterNews) < 0)
		{
			delete *iterNews;
		}
		else
		{
			ret = pop->insert(*iterNews);

			if(ret.second == true)
			{
				nins++;

				if((int)pop->size() > tamPop)
				{
					pointSol = *pop->rbegin();

					pop->erase(pointSol);
					delete pointSol;
				}
			}
			else
			{
				delete *iterNews;
			}
		}
	}

	Problema::best = (*pop->begin())->getFitness();
	Problema::worst = (*pop->rbegin())->getFitness();

	return new pair<int, int>(nret, nins);
}

inline void Controle::geraPop(list<Problema*>* popInicial)
{
	pair<set<Problema*, bool(*)(Problema*, Problema*)>::iterator, bool> ret;

	if(popInicial != NULL)
	{
		for(list<Problema*>::iterator iter = popInicial->begin(); iter != popInicial->end(); iter++)
		{
			if((int)pop->size() < tamPop)
			{
				ret = pop->insert(*iter);
				if(!ret.second)
					delete *iter;
			}
			else
			{
				delete *iter;
			}
		}
	}

	unsigned long int limit = pow(tamPop, 3), iter = 0;
	Problema* soluction = NULL;

	cout << endl << "LOADING: " << flush;

	int loadingMax = 100, loading = ceil((int)pop->size()*loadingMax/tamPop);

	for(int i = 0; i < loading; i++)
		cout << '*' << flush;

	while((int)pop->size() < tamPop && iter < limit && PARAR == false)
	{
		soluction = Problema::randSoluction();

		if(soluction->getFitness() != -1)
		{
			ret = pop->insert(soluction);
			if(!ret.second)
			{
				iter++;

				delete soluction;
			}
			else
			{
				if((ceil((int)pop->size()*loadingMax/tamPop) - loading) == 1)
				{
					cout << '#' << flush;
					loading++;
				}

				if(iter > 0)
					iter--;
			}
		}
		else
		{
			iter++;

			delete soluction;
		}
	}

	cout << " " << loading << "%" << endl << endl;

	PARAR = false;

	return;
}


void Controle::display()
{
	/* Limpa buffer */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Define a posicao da camera */
	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	/* Reinicia o sistema de coordenadas */
	glLoadIdentity();

	/* Restaura a posicao da camera */
	gluLookAt (0, 0, 5, 0, 0, 0, 0, 1, 0);

	/* Desenha as informacoes na tela */
	float linha = 1.4;
	float coluna = -5;
	for(list<list<Heuristica_Listener*>::iterator >::iterator iter = actAlgs->begin(); iter != actAlgs->end(); iter++)
	{
		glColor3f(1.0f, 0.0f, 0.0f);
		Controle::drawstr(coluna, linha+0.4, GLUT_BITMAP_TIMES_ROMAN_24, "%s -> STATUS: %.2f %\n", (**iter)->info.c_str(), (**iter)->status);

		glColor3f(0.0f, 1.0f, 0.0f);
		Controle::drawstr(coluna, linha+0.2, GLUT_BITMAP_HELVETICA_12, "Best Initial Solution: %.0f\t | \tBest Current Solution: %.0f\n\n", (**iter)->bestInitialFitness, (**iter)->bestActualFitness);

		glColor3f(0.0f, 0.0f, 1.0f);
		Controle::drawstr(coluna, linha, GLUT_BITMAP_9_BY_15, (**iter)->getInfo());

		coluna += 3.4;

		if(coluna > 1.8)
		{
			coluna = -5;
			linha -= 1;
		}
	}

	glutSwapBuffers();

	usleep(250000);
}

void Controle::reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)width/height, 0.025, 25.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
}

void Controle::drawstr(GLfloat x, GLfloat y, GLvoid *font_style, const char* format, ...)
{
	if(format == NULL)
		return;

	va_list args;
	char buffer[512], *s;

	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	glRasterPos2f(x, y);

	for (s = buffer; *s; s++)
	  glutBitmapCharacter(font_style, *s);
}


inline bool cmpAlg(Heuristica *h1, Heuristica *h2)
{
	return h1->prob < h2->prob;
}


Controle* Controle::instance = NULL;

list<Heuristica_Listener*>* Controle::execAlgs = NULL;
list<list<Heuristica_Listener*>::iterator >* Controle::actAlgs = NULL;
int Controle::actThreads = 0;

int* Controle::argc = NULL;
char** Controle::argv = NULL;

int Controle::window = 0;


int Heuristica::numHeuristic = 0;
