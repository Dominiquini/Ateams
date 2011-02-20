#include "Controle.h"

using namespace std;

pthread_mutex_t mutex_pop;	// Mutex que protege a populacao principal
pthread_mutex_t mutex_cont;	// Mutex que protege as variaveis de criacao de novas solucoes
pthread_mutex_t mutex_info;	// Mutex que protege a impressao das informacoes da execucao
pthread_mutex_t mutex_exec;	// Mutex que protege a impressao as informacoes de execucao

sem_t semaphore;			// Semaforo que controla o acesso dos algoritmos ao processador

Controle::Controle()
{
	tamPop = 1000;
	critUnicidade = 1;
	iterAteams = 250;
	tentAteams = 100;
	maxTempo = 3600;
	numThreads = 4;
	makespanBest = -1;

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = critUnicidade == 1 ? fncomp1 : fncomp2;
	pop = new set<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	algs = new vector<Heuristica*>;

	execAlgs = new list<Heuristica_Listener*>;
	actAlgs = new list<list<Heuristica_Listener*>::iterator >;
	this->listener = false;
	actThreads = 0;

	pthread_mutex_init(&mutex_pop, NULL);
	pthread_mutex_init(&mutex_cont, NULL);
	pthread_mutex_init(&mutex_info, NULL);
	pthread_mutex_init(&mutex_exec, NULL);

	sem_init(&semaphore, 0, numThreads);
}

Controle::Controle(ParametrosATEAMS* pATEAMS, bool listener)
{
	tamPop = pATEAMS->tamPopAteams != -1 ? pATEAMS->tamPopAteams : 1000;
	critUnicidade = pATEAMS->critUnicidade != -1 ? pATEAMS->critUnicidade : 1;
	iterAteams = pATEAMS->iterAteams != -1 ? pATEAMS->iterAteams: 100;
	tentAteams = pATEAMS->tentAteams != -1 ? pATEAMS->tentAteams: 50;
	maxTempo = pATEAMS->maxTempoAteams != -1 ? pATEAMS->maxTempoAteams : INT_MAX;
	numThreads = pATEAMS->numThreads != -1 ? pATEAMS->numThreads : 4;
	makespanBest = pATEAMS->makespanBest != -1 ? pATEAMS->makespanBest : -1;

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = critUnicidade == 1 ? fncomp1 : fncomp2;
	pop = new set<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	algs = new vector<Heuristica*>;

	execAlgs = new list<Heuristica_Listener*>;
	actAlgs = new list<list<Heuristica_Listener*>::iterator >;
	this->listener = listener;
	actThreads = 0;

	pthread_mutex_init(&mutex_pop, NULL);
	pthread_mutex_init(&mutex_cont, NULL);
	pthread_mutex_init(&mutex_info, NULL);
	pthread_mutex_init(&mutex_exec, NULL);

	sem_init(&semaphore, 0, numThreads);
}

Controle::~Controle()
{
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;

	for(iter = pop->begin(); iter != pop->end(); iter++)
		delete *iter;

	pop->clear();
	delete pop;

	vector<Heuristica*>::reverse_iterator it;

	cout << endl << endl << "Execuções: " << execThreads << endl << endl;
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

set<Problema*, bool(*)(Problema*, Problema*)>::iterator Controle::selectRouletteWheel(set<Problema*, bool(*)(Problema*, Problema*)>* probs, double fitTotal, unsigned int randWheel)
{
	// Armazena o fitness total da populacao
	unsigned int sum = (unsigned int)fitTotal;
	// Um numero entre zero e "sum" e sorteado
	randWheel = xRand(randWheel, 0, sum+1);

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

vector<Problema*>::iterator Controle::selectRouletteWheel(vector<Problema*>* probs, double fitTotal, unsigned int randWheel)
{
	// Armazena o fitness total da populacao
	unsigned int sum = (unsigned int)fitTotal;
	// Um numero entre zero e "sum" e sorteado
	randWheel = xRand(randWheel, 0, sum+1);

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

Heuristica* Controle::selectRouletteWheel(vector<Heuristica*>* heuristc, unsigned int probTotal, unsigned int randWheel)
{
	// Armazena o fitness total da populacao
	unsigned int sum = probTotal;
	// Um numero entre zero e "sum" e sorteado
	randWheel = xRand(randWheel, 0, sum+1);

	for(int i = 0; i < (int)heuristc->size(); i++)
	{
		sum -= heuristc->at(i)->prob;
		if(sum <= randWheel)
		{
			return heuristc->at(i);
		}
	}
	return heuristc->at(0);
}

vector<Problema*>::iterator Controle::selectRandom(vector<Problema*>* probs, int randWheel)
{
	randWheel = randWheel % probs->size();

	vector<Problema*>::iterator iter = probs->begin();
	for(int i = 0; iter != probs->end() && i < randWheel; iter++, i++);

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

void Controle::getInfo(execInfo *info)
{
	info->diffTime = difftime(time2, time1);
	info->numExecs = execThreads;

	info->worstFitness = Problema::worst;
	info->bestFitness = Problema::best;
	info->expSol = Problema::totalNumInst;
}

Problema* Controle::start(list<Problema*>* popInicial)
{
	time(&time1);

	pthread_t *threads = (pthread_t*)malloc(iterAteams * sizeof(pthread_t));
	pthread_t threadTime;
	void* temp = NULL;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	srand(unsigned(time(NULL)));

	geraPop(popInicial);

	if(pop->size() == 0)
	{
		cout << endl << "Nenuma Solução Inicial Encontrada!" << endl << endl;

		exit(1);
	}

	iterMelhora = 0;

	Problema::best = (*pop->begin())->getFitness();
	Problema::worst = (*pop->rbegin())->getFitness();

	cout << endl << "Pior Solução Inicial: " << Problema::worst << endl << endl;
	cout << "Melhor Solução Inicial: " << Problema::best << endl << endl << endl;

	pair<int, Controle*>* par = NULL;
	long int ins = 0;
	execThreads = 0;

	if(listener)
	{
		pthread_t threadAnimation;

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		pthread_create(&threadAnimation, &attr, pthrAnimation, NULL);
	}

	if(pthread_create(&threadTime, &attr, pthrTime, (void*)this) != 0)
	{
		cout << endl << endl << "Erro na criação da Thread! (pthrTime)" << endl << endl;
		exit(1);
	}

	for(int execAteams = 0; execAteams < iterAteams; execAteams++)
	{
		par = new pair<int, Controle*>();
		par->first = execAteams+1;
		par->second = this;

		if(pthread_create(&threads[execAteams], &attr, pthrExec, (void*)par) != 0)
		{
			cout << endl << endl << "Erro na criação da Thread! (pthrExec)" << endl << endl;
			exit(1);
		}
	}

	for(int execAteams = 0; execAteams < iterAteams; execAteams++)
	{
		pthread_join(threads[execAteams], &temp);
		ins += (long int)temp;
	}

	PARAR = true;

	pthread_join(threadTime, NULL);

	cout << endl << "Soluções Permutadas: " << ins << endl;

	free(threads);

	pthread_attr_destroy(&attr);

	time(&time2);

	if(listener)
		glutDestroyWindow(window);

	return Problema::copySoluction(**(pop->begin()));
}

inline int Controle::exec(int randomic, int idThread)
{
	srand(randomic);

	list<Heuristica_Listener*>::iterator listener_iterator;
	vector<Problema*> *newSoluctions = NULL;
	Heuristica_Listener* listener = NULL;
	Heuristica* alg = NULL;
	pair<int, int>* insert;
	string execNames;
	int contrib = 0;


	pthread_mutex_lock(&mutex_info);
	{
		alg = selectRouletteWheel(algs, Heuristica::numHeuristic, rand());
		listener = new Heuristica_Listener(alg, idThread);

		actThreads++;

		listener_iterator = execAlgs->insert(execAlgs->begin(), listener);
		actAlgs->push_back(listener_iterator);

		execNames = "";
		for(list<list<Heuristica_Listener*>::iterator >::iterator it = actAlgs->begin(); it != actAlgs->end(); it++)
			execNames = execNames + (**it)->info + " ";

		printf(">>> ALG: %s | ..................................................... | FILA: (%d : %s)\n", listener->info.c_str(), actThreads, execNames.c_str());
	}
	pthread_mutex_unlock(&mutex_info);

	if(this->listener)
		newSoluctions = alg->start(pop, randomic, listener);
	else
		newSoluctions = alg->start(pop, randomic, NULL);

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

		printf(" | FILA: (%d : %s)\n", actThreads, execNames.c_str());
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
	long int ins = 0;

	delete in;

	sem_wait(&semaphore);

	if(PARAR != true)
	{
		ins = ctr->exec(rand(), execAteams);

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
	glutInitWindowSize(1000, 500);
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

	srand(unsigned(time(NULL)));

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
	float posY = 0;
	for(list<list<Heuristica_Listener*>::iterator >::iterator iter = actAlgs->begin(); iter != actAlgs->end(); iter++)
	{
		glColor3f(1.0f, 0.0f, 0.0f);
		Controle::drawstr(-4, posY+1.8, GLUT_BITMAP_TIMES_ROMAN_24, "%s -> STATUS: %.2f %\n", (**iter)->info.c_str(), (**iter)->status);

		glColor3f(0.0f, 1.0f, 0.0f);
		Controle::drawstr(-4, posY+1.6, GLUT_BITMAP_TIMES_ROMAN_10, "Melhor solucao inicial: %.0f       Melhor solucao atual: %.0f\n\n", (**iter)->bestInitialFitness, (**iter)->bestActualFitness);

		glColor3f(0.0f, 0.0f, 1.0f);
		Controle::drawstr(-4, posY+1.4, GLUT_BITMAP_TIMES_ROMAN_10, (**iter)->getInfo());

		posY -= 1;
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

list<Heuristica_Listener*>* Controle::execAlgs = NULL;
list<list<Heuristica_Listener*>::iterator >* Controle::actAlgs = NULL;
int Controle::actThreads = 0;

int* Controle::argc = NULL;
char** Controle::argv = NULL;

int Controle::window = 0;


int Heuristica::numHeuristic = 0;
