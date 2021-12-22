#include "Control.hpp"

using namespace xercesc;
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

	instance->readMainCMDParameters();

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

	instance->readAdditionalCMDParameters();

	return instance;
}

void Control::terminate() {
	delete instance;
	instance = NULL;
}

Control::Control() {
	this->heuristics = NULL;
	this->solutions = NULL;

	this->populationSize = 500;
	this->comparatorMode = 1;
	this->iterations = 250;
	this->attemptsWithoutImprovement = 100;
	this->maxExecutionTime = 3600;
	this->numThreads = 4;
	this->bestKnownFitness = -1;
	this->printFullSolution = false;
	this->heuristicListener = false;

	this->startTime = this->endTime = 0;
	this->lastImprovedIteration = 0;
	this->executionCount = 0;

	executedHeuristics = new list<HeuristicListener*>;
	runningHeuristics = new list<list<HeuristicListener*>::iterator>;
	runningThreads = 0;

	pthread_mutex_init(&mutex_pop, NULL);
	pthread_mutex_init(&mutex_cont, NULL);
	pthread_mutex_init(&mutex_info, NULL);
	pthread_mutex_init(&mutex_exec, NULL);
}

Control::~Control() {
	set<Problem*, bool (*)(Problem*, Problem*)>::iterator iter;

	for (iter = solutions->begin(); iter != solutions->end(); iter++)
		delete *iter;

	solutions->clear();
	delete solutions;

	vector<Heuristic*>::reverse_iterator it;

	for (it = heuristics->rbegin(); it != heuristics->rend(); it++)
		delete *it;

	heuristics->clear();
	delete heuristics;

	for (list<HeuristicListener*>::iterator it = executedHeuristics->begin(); it != executedHeuristics->end(); it++)
		delete *it;

	executedHeuristics->clear();
	delete executedHeuristics;

	runningHeuristics->clear();
	delete runningHeuristics;

	pthread_mutex_destroy(&mutex_pop);
	pthread_mutex_destroy(&mutex_cont);
	pthread_mutex_destroy(&mutex_info);
	pthread_mutex_destroy(&mutex_exec);

	int window = glutGetWindow();
	if (window != 0)
		glutDestroyWindow(window);
}

int Control::execute(int idThread) {
	list<HeuristicListener*>::iterator listener_iterator;
	vector<Problem*> *newSoluctions = NULL;
	HeuristicListener *listener = NULL;
	Heuristic *algorithm = NULL;
	pair<int, int> *insert;
	string execNames;
	int contrib = 0;

	pthread_mutex_lock(&mutex_info);
	{
		algorithm = selectRouletteWheel(heuristics, Heuristic::heuristicsAvailable);
		listener = new HeuristicListener(algorithm, idThread);

		runningThreads++;

		listener_iterator = executedHeuristics->insert(executedHeuristics->begin(), listener);
		runningHeuristics->push_back(listener_iterator);

		execNames = "";
		for (list<list<HeuristicListener*>::iterator>::iterator it = runningHeuristics->begin(); it != runningHeuristics->end(); it++)
			execNames = execNames + (**it)->info + " ";

		printf(">>> ALG: %s | ..................................................... | QUEUE: (%d : %s)\n", listener->info.c_str(), runningThreads, execNames.c_str());
	}
	pthread_mutex_unlock(&mutex_info);

	if (this->heuristicListener)
		newSoluctions = algorithm->start(solutions, listener);
	else
		newSoluctions = algorithm->start(solutions, NULL);

	pthread_mutex_lock(&mutex_pop);
	{
		double oldBest = Problem::best;

		insert = addSolutions(newSoluctions);
		executionCount++;

		double newBest = Problem::best;

		if (Problem::improvement(oldBest, newBest) > 0)
			lastImprovedIteration = 0;
		else
			lastImprovedIteration++;
	}
	pthread_mutex_unlock(&mutex_pop);

	newSoluctions->clear();
	delete newSoluctions;

	pthread_mutex_lock(&mutex_info);
	{
		runningThreads--;

		list<list<HeuristicListener*>::iterator>::iterator exec = find(runningHeuristics->begin(), runningHeuristics->end(), listener_iterator);

		runningHeuristics->erase(exec);

		execNames = "";
		for (list<list<HeuristicListener*>::iterator>::iterator it = runningHeuristics->begin(); it != runningHeuristics->end(); it++)
			execNames = execNames + (**it)->info + " ";

		printf("<<< ALG: %s | ITER: %.3d | FITNESS: %.6d:%.6d | CONTRIB: %.3d:%.3d | QUEUE: (%d : %s)\n", listener->info.c_str(), executionCount, (int) Problem::best, (int) Problem::worst, insert->first, insert->second, runningThreads, execNames.c_str());
	}
	pthread_mutex_unlock(&mutex_info);

	contrib = insert->second;

	delete insert;

	return contrib;
}

pair<int, int>* Control::addSolutions(vector<Problem*> *news) {
	pair<set<Problem*, bool (*)(Problem*, Problem*)>::iterator, bool> ret;
	vector<Problem*>::const_iterator iterNews;
	int nins = 0, nret = news->size();
	Problem *pointSol = NULL;

	for (iterNews = news->begin(); iterNews != news->end(); iterNews++) {
		if (Problem::improvement(**solutions->rbegin(), **iterNews) < 0) {
			delete *iterNews;
		} else {
			ret = solutions->insert(*iterNews);

			if (ret.second == true) {
				nins++;

				if ((int) solutions->size() > populationSize) {
					pointSol = *solutions->rbegin();

					solutions->erase(pointSol);
					delete pointSol;
				}
			} else {
				delete *iterNews;
			}
		}
	}

	Problem::best = (*solutions->begin())->getFitness();
	Problem::worst = (*solutions->rbegin())->getFitness();

	return new pair<int, int>(nret, nins);
}

void Control::generatePopulation(list<Problem*> *popInicial) {
	pair<set<Problem*, bool (*)(Problem*, Problem*)>::iterator, bool> ret;

	if (popInicial != NULL) {
		for (list<Problem*>::iterator iter = popInicial->begin(); iter != popInicial->end(); iter++) {
			if ((int) solutions->size() < populationSize) {
				ret = solutions->insert(*iter);
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

	int loadingMax = 100, loading = ceil((int) solutions->size() * loadingMax / populationSize);

	for (int i = 0; i < loading; i++)
		cout << '*' << flush;

	while ((int) solutions->size() < populationSize && iter < limit && TERMINATE == false) {
		soluction = Problem::randomSolution();

		if (soluction->getFitness() != -1) {
			ret = solutions->insert(soluction);
			if (!ret.second) {
				iter++;

				delete soluction;
			} else {
				if ((ceil((int) solutions->size() * loadingMax / populationSize) - loading) == 1) {
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

	cout << " (" << solutions->size() << ") " << endl;

	TERMINATE = false;

	return;
}

void Control::startElement(const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname, const Attributes &attrs) {
	char *element = XMLString::transcode(localname);

	if (strcasecmp(element, "Controller") == 0) {
		char *parameter = NULL;
		char *value = NULL;

		for (unsigned int ix = 0; ix < attrs.getLength(); ++ix) {
			parameter = XMLString::transcode(attrs.getQName(ix));
			value = XMLString::transcode(attrs.getValue(ix));

			if (!setParameter(parameter, value)) {
				throw string("Invalid Parameter: ").append(parameter);
			}

			XMLString::release(&parameter);
			XMLString::release(&value);
		}
	} else if (strcasecmp(element, "Heuristics") == 0) {
		heuristics = new vector<Heuristic*>();
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
					throw string("Invalid Parameter: ").append(parameter);
				}

				XMLString::release(&parameter);
				XMLString::release(&value);
			}

			addHeuristic(newHeuristic);
		}
	}

	XMLString::release(&element);
}

void Control::readMainCMDParameters() {
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

	if ((p = findPosArgv(argv, *argc, (char*) "-t")) != -1) {
		strcpy(outputLogFile, argv[p]);

		printf("Log File: '%s'\n", outputLogFile);
	} else {
		outputLogFile[0] = '\0';

		printf("~Log File: ---\n");
	}

	setPrintFullSolution(findPosArgv(argv, *argc, (char*) "-s") != -1);
	setGraphicStatusInfoScreen(findPosArgv(argv, *argc, (char*) "-g") != -1);
}

void Control::readAdditionalCMDParameters() {
	int p = -1;

	if ((p = findPosArgv(argv, *argc, (char*) "--iterations")) != -1)
		setParameter("iterations", argv[p]);

	if ((p = findPosArgv(argv, *argc, (char*) "--numThreads")) != -1)
		setParameter("numThreads", argv[p]);

	if ((p = findPosArgv(argv, *argc, (char*) "--attemptsWithoutImprovement")) != -1)
		setParameter("attemptsWithoutImprovement", argv[p]);

	if ((p = findPosArgv(argv, *argc, (char*) "--maxExecutionTime")) != -1)
		setParameter("maxExecutionTime", argv[p]);

	if ((p = findPosArgv(argv, *argc, (char*) "--populationSizeAteams")) != -1)
		setParameter("populationSizeAteams", argv[p]);

	if ((p = findPosArgv(argv, *argc, (char*) "--comparatorMode")) != -1)
		setParameter("comparatorMode", argv[p]);

	if ((p = findPosArgv(argv, *argc, (char*) "--bestKnownFitness")) != -1)
		setParameter("bestKnownFitness", argv[p]);
}

bool Control::setParameter(const char *parameter, const char *value) {
	int read = EOF;

	if (strcasecmp(parameter, "iterations") == 0) {
		read = sscanf(value, "%d", &iterations);
	} else if (strcasecmp(parameter, "attemptsWithoutImprovement") == 0) {
		read = sscanf(value, "%d", &attemptsWithoutImprovement);
	} else if (strcasecmp(parameter, "maxExecutionTime") == 0) {
		read = sscanf(value, "%d", &maxExecutionTime);
	} else if (strcasecmp(parameter, "numThreads") == 0) {
		read = sscanf(value, "%d", &numThreads);
	} else if (strcasecmp(parameter, "populationSizeAteams") == 0) {
		read = sscanf(value, "%d", &populationSize);
	} else if (strcasecmp(parameter, "comparatorMode") == 0) {
		read = sscanf(value, "%d", &comparatorMode);
	} else if (strcasecmp(parameter, "bestKnownFitness") == 0) {
		read = sscanf(value, "%d", &bestKnownFitness);
	}

	return read != EOF;
}

inline void Control::setPrintFullSolution(bool fullPrint) {
	this->printFullSolution = fullPrint;
}

inline void Control::setGraphicStatusInfoScreen(bool statusInfoScreen) {
	this->heuristicListener = statusInfoScreen;
}

inline int Control::findPosArgv(char **in, int num, char *key) {
	for (int i = 0; i < num; i++) {
		if (!strcmp(in[i], key))
			return i + 1;
	}

	return -1;
}

void Control::addHeuristic(Heuristic *alg) {
	heuristics->push_back(alg);

	sort(heuristics->begin(), heuristics->end(), Heuristic::comparator);
}

Problem* Control::start(list<Problem*> *popInicial) {
	bool (*fn_pt)(Problem*, Problem*) = comparatorMode == 1 ? fncomp1 : fncomp2;
	solutions = new set<Problem*, bool (*)(Problem*, Problem*)>(fn_pt);

	sem_init(&semaphore, 0, numThreads);

	time(&startTime);

	pthread_t *threads = (pthread_t*) malloc(iterations * sizeof(pthread_t));
	pthread_t threadAnimation;
	pthread_t threadTime;

	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	generatePopulation(popInicial);

	if (solutions->size() == 0)
		throw "No Initial Solution Found!";

	lastImprovedIteration = 0;

	Problem::best = (*solutions->begin())->getFitness();
	Problem::worst = (*solutions->rbegin())->getFitness();

	cout << endl << "Worst Initial Solution: " << Problem::worst << endl;
	cout << endl << "Best Initial Solution: " << Problem::best << endl;

	cout << endl;

	pair<int, Control*> *par = NULL;
	long int ins = 0;
	executionCount = 0;

	if (heuristicListener) {
		pthread_attr_t animationAttr;

		pthread_attr_init(&animationAttr);
		pthread_attr_setdetachstate(&animationAttr, PTHREAD_CREATE_DETACHED);

		if (pthread_create(&threadAnimation, &animationAttr, pthrAnimation, NULL) != 0)
			throw "Thread Creation Error! (pthrAnimation)";
	}

	if (pthread_create(&threadTime, &attr, pthrTime, (void*) this) != 0)
		throw "Thread Creation Error! (pthrTime)";

	for (int execAteams = 0; execAteams < iterations; execAteams++) {
		par = new pair<int, Control*>();
		par->first = execAteams + 1;
		par->second = this;

		if (pthread_create(&threads[execAteams], &attr, pthrExec, (void*) par) != 0)
			throw "Thread Creation Error! (pthrExec)";
	}

	void *temp = NULL;

	for (int execAteams = 0; execAteams < iterations; execAteams++) {
		pthread_join(threads[execAteams], &temp);
		ins += (uintptr_t) temp;
	}

	TERMINATE = true;

	pthread_join(threadTime, NULL);

	cout << endl;

	cout << endl << "Explored Solutions: " << Problem::totalNumInst << endl;
	cout << endl << "Swapped Solutions: " << ins << endl;

	cout << endl << "Worst Final Solution: " << Problem::worst << endl;
	cout << endl << "Best Final Solution: " << Problem::best << endl;

	free(threads);

	pthread_attr_destroy(&attr);

	time(&endTime);

	return Problem::copySolution(**(solutions->begin()));
}

list<Problem*>* Control::getSolutions() {
	list<Problem*> *sol = new list<Problem*>();
	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator iter;

	for (iter = solutions->begin(); iter != solutions->end(); iter++)
		sol->push_back(*iter);

	return sol;
}

Problem* Control::getSolution(int n) {
	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator iter = solutions->begin();

	for (int i = 0; i <= n && iter != solutions->end(); iter++);

	return Problem::copySolution(**(--iter));
}

void Control::checkSolutions() {
	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator iter1, iter2;

	/* Testa a memoria principal por solucoes repetidas ou fora de ordem */
	for (iter1 = solutions->begin(); iter1 != solutions->end(); iter1++)
		for (iter2 = iter1; iter2 != solutions->end(); iter2++)
			if ((iter1 != iter2) && (fnequal1(*iter1, *iter2) || fncomp1(*iter2, *iter1)))
				throw "Incorrect Main Memory!";
}

ExecutionInfo Control::getExecutionInfo() {
	ExecutionInfo info;

	info.executionTime = difftime(endTime, startTime);
	info.executionCount = executionCount;

	info.worstFitness = Problem::worst;
	info.bestFitness = Problem::best;
	info.exploredSolutions = Problem::totalNumInst;

	return info;
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
	if (heuristic == NULL || heuristic->size() == 0)
		throw "No Heuristics Defined!";

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

		if (ctr->lastImprovedIteration > ctr->attemptsWithoutImprovement)
			TERMINATE = true;

		if ((ctr->bestKnownFitness != -1 ? Problem::improvement(ctr->bestKnownFitness, Problem::best) : -1) >= 0)
			TERMINATE = true;
	}

	sem_post(&semaphore);

	return (void*) ins; 	//	pthread_exit((void*)ins);
}

void* Control::pthrTime(void *obj) {
	Control *ctr = (Control*) obj;
	time_t rawtime;

	while (TERMINATE == false) {
		time(&rawtime);

		if ((int) difftime(rawtime, ctr->startTime) > ctr->maxExecutionTime)
			TERMINATE = true;

		sleep(1);
	}

	return NULL;			//	pthread_exit(NULL);
}

void* Control::pthrAnimation(void *in) {
	/* Cria a tela */
	glutInit(Control::argc, Control::argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(0, 0);

	glutCreateWindow(Control::argv[0]);

	/* Define as funcoes de desenho */
	glutDisplayFunc(Control::display);
	glutIdleFunc(Control::display);
	glutReshapeFunc(Control::reshape);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glLineWidth(2.0);

	/* Loop principal do programa */
	glutMainLoop();

	return NULL;			//	pthread_exit(NULL);
}

void Control::display() {
	/* Limpa buffer */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Reinicia o sistema de coordenadas */
	glLoadIdentity();

	/* Restaura a posicao da camera */
	gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);

	/* Desenha as informacoes na tela */
	float linha = 1.4;
	float coluna = -5;
	for (list<list<HeuristicListener*>::iterator>::iterator iter = runningHeuristics->begin(); iter != runningHeuristics->end(); iter++) {
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

	usleep(WINDOWS_UPDATE_INTERVAL);
}

void Control::reshape(GLint width, GLint height) {
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

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

list<HeuristicListener*> *Control::executedHeuristics = NULL;
list<list<HeuristicListener*>::iterator> *Control::runningHeuristics = NULL;
int Control::runningThreads = 0;

int *Control::argc = NULL;
char **Control::argv = NULL;

int Heuristic::heuristicsAvailable = 0;
