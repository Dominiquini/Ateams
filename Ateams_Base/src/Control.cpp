#include "Control.hpp"

using namespace std;

pthread_mutex_t mutex_pop;	// Mutex que protege a populacao principal
pthread_mutex_t mutex_cont;	// Mutex que protege as variaveis de criacao de novas solucoes
pthread_mutex_t mutex_info;	// Mutex que protege a impressao das informacoes da execucao
pthread_mutex_t mutex_exec;	// Mutex que protege as informacoes de execucao

sem_t semaphore;			// Semaforo que controla o acesso dos algoritmos ao processador

Control* Control::getInstance(int argc, char **argv) {
	Control::argc = &argc;
	Control::argv = argv;

	if (instance == NULL) {
		instance = new Control();
	} else {
		terminate();

		return getInstance(argc, argv);
	}

	instance->readCMDParameters();
	instance->readAdditionalCMDParameters();

	XMLPlatformUtils::Initialize();

	SAX2XMLReader *parser = XMLReaderFactory::createXMLReader();
	parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
	parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);

	DefaultHandler *defaultHandler = instance;
	parser->setContentHandler(defaultHandler);
	parser->setErrorHandler(defaultHandler);

	parser->parse(instance->getInputParameters());

	delete parser;

	XMLPlatformUtils::Terminate();

	return instance;
}

void Control::terminate() {
	delete instance;
	instance = NULL;
}

Control::Control() {
	this->algs = NULL;
	this->pop = NULL;

	this->populationSize = 500;
	this->comparatorMode = 1;
	this->iterAteams = 250;
	this->tentAteams = 100;
	this->maxTime = 3600;
	this->numThreads = 4;
	this->makespanBest = -1;
	this->printFullSolution = false;
	this->activeListener = false;

	this->time1 = this->time2 = 0;
	this->iterMelhora = 0;
	this->execThreads = 0;

	execAlgs = new list<HeuristicListener*>;
	actAlgs = new list<list<HeuristicListener*>::iterator>;
	actThreads = 0;

	pthread_mutex_init(&mutex_pop, NULL);
	pthread_mutex_init(&mutex_cont, NULL);
	pthread_mutex_init(&mutex_info, NULL);
	pthread_mutex_init(&mutex_exec, NULL);
}

Control::~Control() {
	set<Problem*, bool (*)(Problem*, Problem*)>::iterator iter;

	for (iter = pop->begin(); iter != pop->end(); iter++)
		delete *iter;

	pop->clear();
	delete pop;

	vector<Heuristic*>::reverse_iterator it;

	for (it = algs->rbegin(); it != algs->rend(); it++)
		delete *it;

	algs->clear();
	delete algs;

	for (list<HeuristicListener*>::iterator it = execAlgs->begin(); it != execAlgs->end(); it++)
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

inline int Control::execute(int idThread) {
	list<HeuristicListener*>::iterator listener_iterator;
	vector<Problem*> *newSoluctions = NULL;
	HeuristicListener *listener = NULL;
	Heuristic *alg = NULL;
	pair<int, int> *insert;
	string execNames;
	int contrib = 0;

	pthread_mutex_lock(&mutex_info);
	{
		alg = selectRouletteWheel(algs, Heuristic::heuristicsAvailable);
		listener = new HeuristicListener(alg, idThread);

		actThreads++;

		listener_iterator = execAlgs->insert(execAlgs->begin(), listener);
		actAlgs->push_back(listener_iterator);

		execNames = "";
		for (list<list<HeuristicListener*>::iterator>::iterator it = actAlgs->begin(); it != actAlgs->end(); it++)
			execNames = execNames + (**it)->info + " ";

		printf(">>> ALG: %s | ..................................................... | QUEUE: (%d : %s)\n", listener->info.c_str(), actThreads, execNames.c_str());
	}
	pthread_mutex_unlock(&mutex_info);

	if (this->activeListener)
		newSoluctions = alg->start(pop, listener);
	else
		newSoluctions = alg->start(pop, NULL);

	pthread_mutex_lock(&mutex_pop);
	{
		double oldBest = Problem::best;

		insert = addSolutions(newSoluctions);
		execThreads++;

		double newBest = Problem::best;

		if (Problem::improvement(oldBest, newBest) > 0)
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

		list<list<HeuristicListener*>::iterator>::iterator exec = find(actAlgs->begin(), actAlgs->end(), listener_iterator);

		actAlgs->erase(exec);

		printf("<<< ALG: %s | ITER: %.3d | FITNESS: %.6d:%.6d | CONTRIB: %.3d:%.3d", listener->info.c_str(), execThreads, (int) Problem::best, (int) Problem::worst, insert->first, insert->second);

		execNames = "";
		for (list<list<HeuristicListener*>::iterator>::iterator it = actAlgs->begin(); it != actAlgs->end(); it++)
			execNames = execNames + (**it)->info + " ";

		printf(" | QUEUE: (%d : %s)\n", actThreads, execNames.c_str());
	}
	pthread_mutex_unlock(&mutex_info);

	contrib = insert->second;

	delete insert;

	return contrib;
}

inline pair<int, int>* Control::addSolutions(vector<Problem*> *news) {
	pair<set<Problem*, bool (*)(Problem*, Problem*)>::iterator, bool> ret;
	vector<Problem*>::const_iterator iterNews;
	int nins = 0, nret = news->size();
	Problem *pointSol = NULL;

	for (iterNews = news->begin(); iterNews != news->end(); iterNews++) {
		if (Problem::improvement(**pop->rbegin(), **iterNews) < 0) {
			delete *iterNews;
		} else {
			ret = pop->insert(*iterNews);

			if (ret.second == true) {
				nins++;

				if ((int) pop->size() > populationSize) {
					pointSol = *pop->rbegin();

					pop->erase(pointSol);
					delete pointSol;
				}
			} else {
				delete *iterNews;
			}
		}
	}

	Problem::best = (*pop->begin())->getFitness();
	Problem::worst = (*pop->rbegin())->getFitness();

	return new pair<int, int>(nret, nins);
}

inline void Control::generatePopulation(list<Problem*> *popInicial) {
	pair<set<Problem*, bool (*)(Problem*, Problem*)>::iterator, bool> ret;

	if (popInicial != NULL) {
		for (list<Problem*>::iterator iter = popInicial->begin(); iter != popInicial->end(); iter++) {
			if ((int) pop->size() < populationSize) {
				ret = pop->insert(*iter);
				if (!ret.second)
					delete *iter;
			} else {
				delete *iter;
			}
		}
	}

	unsigned long int limit = pow(populationSize, 3), iter = 0;
	Problem *soluction = NULL;

	cout << endl << "LOADING: " << flush;

	int loadingMax = 100, loading = ceil((int) pop->size() * loadingMax / populationSize);

	for (int i = 0; i < loading; i++)
		cout << '*' << flush;

	while ((int) pop->size() < populationSize && iter < limit && TERMINATE == false) {
		soluction = Problem::randomSolution();

		if (soluction->getFitness() != -1) {
			ret = pop->insert(soluction);
			if (!ret.second) {
				iter++;

				delete soluction;
			} else {
				if ((ceil((int) pop->size() * loadingMax / populationSize) - loading) == 1) {
					cout << '#' << flush;
					loading++;
				}

				if (iter > 0)
					iter--;
			}
		} else {
			iter++;

			delete soluction;
		}
	}

	cout << " " << loading << "%" << endl << endl;

	TERMINATE = false;

	return;
}

inline void Control::startElement(const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname, const Attributes &attrs) {
	char *element = XMLString::transcode(localname);

	if (strcasecmp(element, "Controller") == 0) {
		char *parameter = NULL;
		char *value = NULL;

		for (unsigned int ix = 0; ix < attrs.getLength(); ++ix) {
			parameter = XMLString::transcode(attrs.getQName(ix));
			value = XMLString::transcode(attrs.getValue(ix));

			if (!setParameter(parameter, value)) {
				throw string("Invalid Parameter: \" ").append(parameter).append(" \"");
			}

			XMLString::release(&parameter);
			XMLString::release(&value);
		}
	} else if (strcasecmp(element, "Heuristics") == 0) {
		algs = new vector<Heuristic*>();
	} else {
		Heuristic *newHeuristic = NULL;

		if (strcasecmp(element, "SimulatedAnnealing") == 0)
			newHeuristic = new SimulatedAnnealing();

		if (strcasecmp(element, "TabuSearch") == 0)
			newHeuristic = new TabuSearch();

		if (strcasecmp(element, "GeneticAlgorithm") == 0)
			newHeuristic = new GeneticAlgorithm();

		if (newHeuristic != NULL) {
			char *parameter = NULL;
			char *value = NULL;

			for (unsigned int ix = 0; ix < attrs.getLength(); ++ix) {
				parameter = XMLString::transcode(attrs.getQName(ix));
				value = XMLString::transcode(attrs.getValue(ix));

				if (!newHeuristic->setParameter(parameter, value)) {
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

inline void Control::readCMDParameters() {
	int p = -1;

	if ((p = findPosArgv(argv, *argc, (char*) "-p")) != -1) {
		strcpy(inputParameters, argv[p]);

		printf("Parameters File: '%s'\n", inputParameters);
	} else {
		inputParameters[0] = '\0';

		printf("Parameters File Cannot Be Empty!\n");

		printf("\n./Ateams -i <<INPUT_FILE>> -p <<INPUT_PARAMETERS>> -r <RESULT_FILE> -l <LOG_FILE>\n\n");

		exit(1);
	}

	if ((p = findPosArgv(argv, *argc, (char*) "-i")) != -1) {
		strcpy(inputDataFile, argv[p]);

		printf("Data File: '%s'\n", inputDataFile);
	} else {
		inputDataFile[0] = '\0';

		printf("Data File Cannot Be Empty!\n");

		printf("\n./Ateams -i <<INPUT_FILE>> -p <<INPUT_PARAMETERS>> -r <RESULT_FILE> -l <LOG_FILE>\n\n");

		exit(1);
	}

	if ((p = findPosArgv(argv, *argc, (char*) "-r")) != -1) {
		strcpy(outputResultFile, argv[p]);

		printf("Result File: '%s'\n", outputResultFile);
	} else {
		outputResultFile[0] = '\0';

		printf("~Result File: ---\n");
	}

	if ((p = findPosArgv(argv, *argc, (char*) "-l")) != -1) {
		strcpy(outputLogFile, argv[p]);

		printf("Log File: '%s'\n", outputLogFile);
	} else {
		outputLogFile[0] = '\0';

		printf("~Log File: ---\n");
	}

	setPrintFullSolution(findPosArgv(argv, *argc, (char*) "-d") != -1);
	setGraphicStatusInfoScreen(findPosArgv(argv, *argc, (char*) "-g") != -1);
}

void Control::readAdditionalCMDParameters() {
	int p = -1;

	if ((p = findPosArgv(argv, *argc, (char*) "--iterAteams")) != -1)
		setParameter("iterAteams", argv[p]);

	if ((p = findPosArgv(argv, *argc, (char*) "--numThreads")) != -1)
		setParameter("numThreads", argv[p]);

	if ((p = findPosArgv(argv, *argc, (char*) "--tentAteams")) != -1)
		setParameter("tentAteams", argv[p]);

	if ((p = findPosArgv(argv, *argc, (char*) "--maxTimeAteams")) != -1)
		setParameter("maxTimeAteams", argv[p]);

	if ((p = findPosArgv(argv, *argc, (char*) "--populationSizeAteams")) != -1)
		setParameter("populationSizeAteams", argv[p]);

	if ((p = findPosArgv(argv, *argc, (char*) "--comparatorMode")) != -1)
		setParameter("comparatorMode", argv[p]);

	if ((p = findPosArgv(argv, *argc, (char*) "--makespanBest")) != -1)
		setParameter("makespanBest", argv[p]);
}

bool Control::setParameter(const char *parameter, const char *value) {
	if (strcasecmp(parameter, "iterAteams") == 0) {
		sscanf(value, "%d", &iterAteams);
	} else if (strcasecmp(parameter, "tentAteams") == 0) {
		sscanf(value, "%d", &tentAteams);
	} else if (strcasecmp(parameter, "maxTimeAteams") == 0) {
		sscanf(value, "%d", &maxTime);
	} else if (strcasecmp(parameter, "numThreads") == 0) {
		sscanf(value, "%d", &numThreads);
	} else if (strcasecmp(parameter, "populationSizeAteams") == 0) {
		sscanf(value, "%d", &populationSize);
	} else if (strcasecmp(parameter, "comparatorMode") == 0) {
		sscanf(value, "%d", &comparatorMode);
	} else if (strcasecmp(parameter, "makespanBest") == 0) {
		sscanf(value, "%d", &makespanBest);
	} else {
		return false;
	}

	return true;
}

inline void Control::setPrintFullSolution(bool fullPrint) {
	this->printFullSolution = fullPrint;
}

inline void Control::setGraphicStatusInfoScreen(bool statusInfoScreen) {
	this->activeListener = statusInfoScreen;
}

inline int Control::findPosArgv(char **in, int num, char *key) {
	for (int i = 0; i < num; i++) {
		if (!strcmp(in[i], key))
			return i + 1;
	}

	return -1;
}

void Control::addHeuristic(Heuristic *alg) {
	algs->push_back(alg);

	sort(algs->begin(), algs->end(), cmpAlg);
}

Problem* Control::start(list<Problem*> *popInicial) {
	bool (*fn_pt)(Problem*, Problem*) = comparatorMode == 1 ? fncomp1 : fncomp2;
	pop = new set<Problem*, bool (*)(Problem*, Problem*)>(fn_pt);

	sem_init(&semaphore, 0, numThreads);

	time(&time1);

	pthread_t *threads = (pthread_t*) malloc(iterAteams * sizeof(pthread_t));
	pthread_t threadTime;
	void *temp = NULL;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	generatePopulation(popInicial);

	if (pop->size() == 0) {
		cout << endl << "No Initial Solution Found!" << endl << endl;

		exit(1);
	}

	iterMelhora = 0;

	Problem::best = (*pop->begin())->getFitness();
	Problem::worst = (*pop->rbegin())->getFitness();

	cout << endl << "Worst Initial Solution: " << Problem::worst << endl << endl;
	cout << "Best Initial Solution: " << Problem::best << endl << endl << endl;

	pair<int, Control*> *par = NULL;
	long int ins = 0;
	execThreads = 0;

	if (activeListener) {
		pthread_t threadAnimation;

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		pthread_create(&threadAnimation, &attr, pthrAnimation, NULL);
	}

	if (pthread_create(&threadTime, &attr, pthrTime, (void*) this) != 0) {
		cout << endl << endl << "Thread Creation Error! (pthrTime)" << endl << endl;
		exit(1);
	}

	for (int execAteams = 0; execAteams < iterAteams; execAteams++) {
		par = new pair<int, Control*>();
		par->first = execAteams + 1;
		par->second = this;

		if (pthread_create(&threads[execAteams], &attr, pthrExec, (void*) par) != 0) {
			cout << endl << endl << "Thread Creation Error! (pthrExec)" << endl << endl;
			exit(1);
		}
	}

	for (int execAteams = 0; execAteams < iterAteams; execAteams++) {
		pthread_join(threads[execAteams], &temp);
		ins += (uintptr_t) temp;
	}

	TERMINATE = true;

	pthread_join(threadTime, NULL);

	cout << endl << "Swapped Solutions: " << ins << endl;

	free(threads);

	pthread_attr_destroy(&attr);

	time(&time2);

	if (activeListener)
		glutDestroyWindow(window);

	return Problem::copySolution(**(pop->begin()));
}

int Control::getExecutions() {
	return execThreads;
}

list<Problem*>* Control::getSolutions() {
	list<Problem*> *sol = new list<Problem*>();
	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator iter;

	for (iter = pop->begin(); iter != pop->end(); iter++)
		sol->push_back(*iter);

	return sol;
}

Problem* Control::getSolution(int n) {
	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator iter = pop->begin();

	for (int i = 0; i <= n && iter != pop->end(); iter++);

	return Problem::copySolution(**(--iter));
}

void Control::getInfo(ExecInfo *info) {
	info->diffTime = difftime(time2, time1);
	info->numExecs = execThreads;

	info->worstFitness = Problem::worst;
	info->bestFitness = Problem::best;
	info->expSol = Problem::totalNumInst;
}

void Control::checkSolutions() {
	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator iter1, iter2;

	/* Testa a memoria principal por solucoes repetidas ou fora de ordem */
	for (iter1 = pop->begin(); iter1 != pop->end(); iter1++)
		for (iter2 = iter1; iter2 != pop->end(); iter2++)
			if ((iter1 != iter2) && (fnequal1(*iter1, *iter2) || fncomp1(*iter2, *iter1)))
				throw "Incorrect Main Memory!";
}

void Control::printSolution(Problem *solution) {
	solution->print(printFullSolution);
}

double Control::sumFitnessMaximize(set<Problem*, bool (*)(Problem*, Problem*)> *probs, int n) {
	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator iter;
	double sum = 0, i = 0;

	for (i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++)
		sum += (*iter)->getFitnessMaximize();

	return sum;
}

double Control::sumFitnessMaximize(vector<Problem*> *probs, int n) {
	vector<Problem*>::const_iterator iter;
	double sum = 0, i = 0;

	for (i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++)
		sum += (*iter)->getFitnessMaximize();

	return sum;
}

double Control::sumFitnessMinimize(set<Problem*, bool (*)(Problem*, Problem*)> *probs, int n) {
	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator iter;
	double sum = 0, i = 0;

	for (i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++)
		sum += (*iter)->getFitnessMinimize();

	return sum;
}

double Control::sumFitnessMinimize(vector<Problem*> *probs, int n) {
	vector<Problem*>::const_iterator iter;
	double sum = 0, i = 0;

	for (i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++)
		sum += (*iter)->getFitnessMinimize();

	return sum;
}

set<Problem*, bool (*)(Problem*, Problem*)>::iterator Control::selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *probs, double fitTotal) {
	// Armazena o fitness total da populacao
	unsigned int sum = (unsigned int) fitTotal;
	// Um numero entre zero e "sum" e sorteado
	unsigned int randWheel = xRand(0, sum + 1);

	set<Problem*, bool (*)(Problem*, Problem*)>::iterator iter;
	for (iter = probs->begin(); iter != probs->end(); iter++) {
		sum -= (int) (*iter)->getFitnessMaximize();
		if (sum <= randWheel) {
			return iter;
		}
	}
	return (probs->begin());
}

vector<Problem*>::iterator Control::selectRouletteWheel(vector<Problem*> *probs, double fitTotal) {
	// Armazena o fitness total da populacao
	unsigned int sum = (unsigned int) fitTotal;
	// Um numero entre zero e "sum" e sorteado
	unsigned int randWheel = xRand(0, sum + 1);

	vector<Problem*>::iterator iter;
	for (iter = probs->begin(); iter != probs->end(); iter++) {
		sum -= (int) (*iter)->getFitnessMaximize();
		if (sum <= randWheel) {
			return iter;
		}
	}
	return probs->begin();
}

Heuristic* Control::selectRouletteWheel(vector<Heuristic*> *heuristic, unsigned int probTotal) {
	if (heuristic == NULL || heuristic->size() == 0) {
		cout << "No Heuristics Defined!" << endl << endl;

		exit(1);
	}

	// Armazena o fitness total da populacao
	unsigned int sum = probTotal;
	// Um numero entre zero e "sum" e sorteado
	unsigned int randWheel = xRand(0, sum + 1);

	for (int i = 0; i < (int) heuristic->size(); i++) {
		sum -= heuristic->at(i)->choiceProbability;
		if (sum <= randWheel) {
			return heuristic->at(i);
		}
	}
	return heuristic->at(0);
}

vector<Problem*>::iterator Control::selectRandom(vector<Problem*> *probs) {
	unsigned int randWheel = xRand(0, probs->size());

	vector<Problem*>::iterator iter = probs->begin();
	for (unsigned long i = 0; iter != probs->end() && i < randWheel; iter++, i++);

	return iter;
}

list<Problem*>::iterator Control::findSol(list<Problem*> *vect, Problem *p) {
	list<Problem*>::iterator iter;

	for (iter = vect->begin(); iter != vect->end(); iter++)
		if (fnequal1((*iter), p))
			return iter;

	return iter;
}

void* Control::pthrExec(void *obj) {
	pair<int, Control*> *in = (pair<int, Control*>*) obj;
	int execAteams = in->first;
	Control *ctr = in->second;
	intptr_t ins = 0;

	delete in;

	sem_wait(&semaphore);

	if (TERMINATE != true) {
		ins = ctr->execute(execAteams);

		if (ctr->iterMelhora > ctr->tentAteams || (ctr->makespanBest != -1 && Problem::improvement(ctr->makespanBest, Problem::best) >= 0))
			TERMINATE = true;
	}

	sem_post(&semaphore);

	return (void*) ins; 		//	pthread_exit((void*)ins);
}

void* Control::pthrTime(void *obj) {
	Control *ctr = (Control*) obj;
	time_t rawtime;

	while (TERMINATE == false) {
		time(&rawtime);

		if ((int) difftime(rawtime, ctr->time1) > ctr->maxTime)
			TERMINATE = true;

		sleep(1);
	}

	return NULL;			//	pthread_exit(NULL);
}

void* Control::pthrAnimation(void *in) {
	/* Cria a tela */
	glutInit(Control::argc, Control::argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(1250, 500);
	glutInitWindowPosition(0, 0);
	window = glutCreateWindow(Control::argv[0]);

	/* Define as funcoes de desenho */
	glutDisplayFunc(Control::display);
	glutIdleFunc(Control::display);
	glutReshapeFunc(Control::reshape);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0);

	/* Loop principal do programa */
	glutMainLoop();

	return NULL;			//	pthread_exit(NULL);
}

void Control::display() {
	/* Limpa buffer */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Define a posicao da camera */
	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	/* Reinicia o sistema de coordenadas */
	glLoadIdentity();

	/* Restaura a posicao da camera */
	gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);

	/* Desenha as informacoes na tela */
	float linha = 1.4;
	float coluna = -5;
	for (list<list<HeuristicListener*>::iterator>::iterator iter = actAlgs->begin(); iter != actAlgs->end(); iter++) {
		glColor3f(1.0f, 0.0f, 0.0f);
		Control::drawstr(coluna, linha + 0.4, GLUT_BITMAP_TIMES_ROMAN_24, "%s -> STATUS: %.2f %\n", (**iter)->info.c_str(), (**iter)->status);

		glColor3f(0.0f, 1.0f, 0.0f);
		Control::drawstr(coluna, linha + 0.2, GLUT_BITMAP_HELVETICA_12, "Best Initial Solution: %.0f\t | \tBest Current Solution: %.0f\n\n", (**iter)->bestInitialFitness, (**iter)->bestActualFitness);

		glColor3f(0.0f, 0.0f, 1.0f);
		Control::drawstr(coluna, linha, GLUT_BITMAP_9_BY_15, (**iter)->getInfo());

		coluna += 3.4;

		if (coluna > 1.8) {
			coluna = -5;
			linha -= 1;
		}
	}

	glutSwapBuffers();

	usleep(250000);
}

void Control::reshape(int width, int height) {
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float) width / height, 0.025, 25.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
}

void Control::drawstr(GLfloat x, GLfloat y, GLvoid *font_style, const char *format, ...) {
	if (format == NULL)
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

Control *Control::instance = NULL;

list<HeuristicListener*> *Control::execAlgs = NULL;
list<list<HeuristicListener*>::iterator> *Control::actAlgs = NULL;
int Control::actThreads = 0;

int *Control::argc = NULL;
char **Control::argv = NULL;

int Control::window = 0;

int Heuristic::heuristicsAvailable = 0;
