#include "Control.hpp"

using namespace std;

pthread_mutexattr_t mutex_attr;

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

	XMLParser *parser = new XMLParser(instance);
	parser->parseXML(instance->getInputParameters());
	delete parser;

	instance->readAdditionalCMDParameters();

	return instance;
}

void Control::terminate() {
	delete instance;
	instance = NULL;

	Problem::deallocateMemory();

	if (Problem::numInst != 0)
		throw "Memory Leak!";
}

Heuristic* Control::instantiateHeuristic(char *name) {
	if (strcmpi(name, SIMULATED_ANNEALING_NAME) == 0)
		return new SimulatedAnnealing();

	if (strcmpi(name, GENETIC_ALGORITHM_NAME) == 0)
		return new GeneticAlgorithm();

	if (strcmpi(name, TABU_SEARCH_NAME) == 0)
		return new TabuSearch();

	return NULL;
}

Control::Control() {
	this->printFullSolution = false;

	this->showTextOverview = false;
	this->showGraphicalOverview = false;

	this->solutions = new set<Problem*, bool (*)(Problem*, Problem*)>(fnSortSolution);

	this->heuristics = new vector<Heuristic*>();

	this->loadingProgressBar = new ProgressBar("LOADING: ");
	this->executionProgressBar = new ProgressBar("EXECUTING: ");

	this->graphicalOverview = new GraphicalOverview(Control::argc, Control::argv);

	this->startTime = this->endTime = 0;
	this->lastImprovedIteration = 0;
	this->executionCount = 0;
	this->swappedSolutions = 0;
}

Control::~Control() {
	for (set<Problem*>::iterator iter = solutions->begin(); iter != solutions->end(); iter++)
		delete *iter;

	solutions->clear();
	delete solutions;

	for (vector<Heuristic*>::reverse_iterator it = heuristics->rbegin(); it != heuristics->rend(); it++) {
		delete *it;
	}

	heuristics->clear();
	delete heuristics;

	for (list<HeuristicExecutionInfo*>::iterator it = Heuristic::executedHeuristics->begin(); it != Heuristic::executedHeuristics->end(); it++)
		delete *it;

	delete loadingProgressBar;
	delete executionProgressBar;

	delete graphicalOverview;

	Heuristic::executedHeuristics->clear();
	delete Heuristic::executedHeuristics;

	Heuristic::runningHeuristics->clear();
	delete Heuristic::runningHeuristics;
}

int Control::execute(unsigned int executionId) {
	vector<Problem*> *newSoluctions = NULL;
	HeuristicExecutionInfo *listener = NULL;
	pair<int, int> *insertion = NULL;
	Heuristic *algorithm = NULL;
	int contrib = 0;

	pthread_mutex_lock(&mutex_info);
	{
		algorithm = selectRouletteWheel(heuristics, Heuristic::heuristicsProbabilitySum);
		listener = new HeuristicExecutionInfo(algorithm, executionId, pthread_self());

		runningThreads++;

		Heuristic::executedHeuristics->push_back(listener);
		Heuristic::runningHeuristics->push_back(listener);

		Control::printProgress(listener, insertion);
	}
	pthread_mutex_unlock(&mutex_info);

	newSoluctions = algorithm->start(solutions, listener);

	pthread_mutex_lock(&mutex_pop);
	{
		double oldBest = Problem::best;

		insertion = addSolutions(newSoluctions);
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

		list<HeuristicExecutionInfo*>::iterator executed = find(Heuristic::runningHeuristics->begin(), Heuristic::runningHeuristics->end(), listener);
		Heuristic::runningHeuristics->erase(executed);

		Control::printProgress(listener, insertion);
	}
	pthread_mutex_unlock(&mutex_info);

	contrib = insertion->second;

	delete insertion;

	return contrib;
}

pair<int, int>* Control::addSolutions(vector<Problem*> *newSolutions) {
	pair<set<Problem*>::iterator, bool> insertion;
	int nins = 0, nret = newSolutions->size();
	Problem *worstSolution = NULL;

	for (vector<Problem*>::const_iterator iterNews = newSolutions->begin(); iterNews != newSolutions->end(); iterNews++) {
		worstSolution = *solutions->rbegin();

		if (Problem::improvement(*worstSolution, **iterNews) < 0) {
			delete *iterNews;
		} else {
			insertion = solutions->insert(*iterNews);

			if (insertion.second) {
				nins++;

				if ((int) solutions->size() > parameters.populationSize) {
					worstSolution = *solutions->rbegin();

					solutions->erase(worstSolution);
					delete worstSolution;
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
	cout << endl;

	loadingProgressBar->init(parameters.populationSize);

	if (popInicial != NULL) {
		for (list<Problem*>::iterator iter = popInicial->begin(); iter != popInicial->end(); iter++) {
			if ((int) solutions->size() >= parameters.populationSize || !solutions->insert(*iter).second) {
				delete *iter;
			}
		}
	}

	loadingProgressBar->update(solutions->size());

	Problem *soluction = NULL;
	unsigned long int limit = pow(parameters.populationSize, 3), failedAttempts = 0;

	while ((int) solutions->size() < parameters.populationSize && failedAttempts < limit && STATUS == EXECUTING) {
		soluction = Problem::randomSolution();

		if (soluction->getFitness() == -1 || !solutions->insert(soluction).second) {
			failedAttempts++;

			delete soluction;
		}

		loadingProgressBar->update(solutions->size());
	}

	loadingProgressBar->end();

	cout << endl;

	return;
}

inline int Control::findPosArgv(char **in, int num, char *key) {
	for (int i = 0; i < num; i++) {
		if (!strcmp(in[i], key))
			return i + 1;
	}

	return -1;
}

inline void Control::readMainCMDParameters() {
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

	setCMDStatusInfoScreen(findPosArgv(argv, *argc, (char*) "-c") != -1);
	setGraphicStatusInfoScreen(findPosArgv(argv, *argc, (char*) "-g") != -1);
}

inline void Control::readAdditionalCMDParameters() {
	int p = -1;

	for (map<string, void*>::const_iterator param = parameters.keys.begin(); param != parameters.keys.end(); param++) {
		string cmdParameter = COMMAND_LINE_PARAMETER_SUFFIX + param->first;

		if ((p = findPosArgv(argv, *argc, const_cast<char*>(cmdParameter.c_str()))) != -1) {
			setParameter(param->first.c_str(), argv[p]);
		}
	}
}

inline void Control::setPrintFullSolution(bool fullPrint) {
	this->printFullSolution = fullPrint;
}

inline void Control::setCMDStatusInfoScreen(bool showCMDOverview) {
	this->showTextOverview = showCMDOverview;
}

inline void Control::setGraphicStatusInfoScreen(bool showGraphicalOverview) {
	this->showGraphicalOverview = showGraphicalOverview;
}

void Control::init() {
	pthread_mutexattr_init(&mutex_attr);
	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init(&mutex_pop, &mutex_attr);
	pthread_mutex_init(&mutex_cont, &mutex_attr);
	pthread_mutex_init(&mutex_info, &mutex_attr);
	pthread_mutex_init(&mutex_exec, &mutex_attr);

	sem_init(&semaphore, 0, parameters.numThreads);

	/* Leitura dos dados passados por arquivos */
	Problem::readProblemFromFile(getInputDataFile());

	/* Le memoria prinipal do disco, se especificado */
	list<Problem*> *popInicial = Problem::readPopulationFromLog(getOutputLogFile());

	generatePopulation(popInicial);

	delete popInicial;

	Problem::best = (*solutions->begin())->getFitness();
	Problem::worst = (*solutions->rbegin())->getFitness();

	cout << COLOR_CYAN;

	cout << endl << "Initial Population Size: : " << solutions->size() << endl;
	cout << endl << "Worst Initial Solution: " << Problem::worst << endl;
	cout << endl << "Best Initial Solution: " << Problem::best << endl;

	cout << endl << COLOR_DEFAULT;
}

void Control::finish() {
	cout << endl << COLOR_CYAN;

	cout << endl << "Final Population Size: : " << solutions->size() << endl;
	cout << endl << "Worst Final Solution: " << Problem::worst << endl;
	cout << endl << "Best Final Solution: " << Problem::best << endl;

	cout << COLOR_DEFAULT;

	list<Problem*> *finalSolutions = getSolutions();

	/* Escreve memoria principal no disco */
	Problem::writeCurrentPopulationInLog(getOutputLogFile(), finalSolutions);

	/* Escreve solucao em arquivo no disco */
	Problem::writeResultInFile(getInputDataFile(), getInputParameters(), getExecutionInfo(), getOutputResultFile());

	delete finalSolutions;

	/* Testa a memoria principal por solucoes repetidas ou fora de ordem */
	for (set<Problem*>::const_iterator iter1 = solutions->begin(); iter1 != solutions->end(); iter1++)
		for (set<Problem*>::const_iterator iter2 = iter1; iter2 != solutions->end(); iter2++)
			if ((iter1 != iter2) && (fnEqualSolution(*iter1, *iter2) || fnSortSolution(*iter2, *iter1)))
				throw "Incorrect Main Memory!";

	sem_destroy(&semaphore);

	pthread_mutexattr_destroy(&mutex_attr);

	pthread_mutex_destroy(&mutex_pop);
	pthread_mutex_destroy(&mutex_cont);
	pthread_mutex_destroy(&mutex_info);
	pthread_mutex_destroy(&mutex_exec);
}

void Control::run() {
	time(&startTime);

	if (showTextOverview) {
		cout << endl << flush;
	}

	pthread_t *threads = (pthread_t*) malloc(parameters.iterations * sizeof(pthread_t));
	pthread_t threadManagement;

	pthread_attr_t attrJoinable;
	pthread_attr_init(&attrJoinable);
	pthread_attr_setdetachstate(&attrJoinable, PTHREAD_CREATE_JOINABLE);

	if (solutions->size() == 0) {
		throw "No Initial Solution Found!";
	}

	if (!showTextOverview) {
		executionProgressBar->init(parameters.iterations);
	}

	if (showGraphicalOverview) {
		graphicalOverview->run();
	}

	if (pthread_create(&threadManagement, &attrJoinable, Control::pthrManagement, NULL) != 0)
		throw "Thread Creation Error! (pthrManagement)";

	for (int execAteams = 0; execAteams < parameters.iterations; execAteams++) {
		PrimitiveWrapper<unsigned int> *iteration = new PrimitiveWrapper<unsigned int>(execAteams + 1);
		if (pthread_create(&threads[execAteams], &attrJoinable, Control::pthrExecution, (void*) iteration) != 0)
			throw "Thread Creation Error! (pthrExecution)";
	}

	for (uintptr_t execAteams = 0, *inserted = NULL; execAteams < (uintptr_t) parameters.iterations; execAteams++) {
		pthread_join(threads[execAteams], (void**) &inserted);

		swappedSolutions += (uintptr_t) inserted;
	}

	if (STATUS == EXECUTING)
		STATUS = (executionCount == parameters.iterations && (int) Heuristic::executedHeuristics->size() == parameters.iterations) ? FINISHED_NORMALLY : INCOMPLETE;

	pthread_join(threadManagement, NULL);

	free(threads);

	pthread_attr_destroy(&attrJoinable);

	executionProgressBar->end();

	if (showTextOverview) {
		cout << PREVIOUS_LINE << flush;
	}

	time(&endTime);
}

list<Problem*>* Control::getSolutions() {
	list<Problem*> *sol = new list<Problem*>();
	set<Problem*>::const_iterator iter;

	for (iter = solutions->begin(); iter != solutions->end(); iter++)
		sol->push_back(*iter);

	return sol;
}

Problem* Control::getSolution(int n) {
	set<Problem*>::const_iterator iter = next(solutions->begin(), n);

	return *iter;
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

void Control::printSolution() {
	Problem *solution = getSolution(0);

	cout << COLOR_MAGENTA;

	cout << endl << endl << "Best Solution: " << solution->getFitness() << endl << endl;

	solution->print(printFullSolution);

	cout << COLOR_DEFAULT;
}

void Control::printExecution() {
	cout << endl << COLOR_BLUE;

	cout << endl << "Explored Solutions: " << Problem::totalNumInst << endl;
	cout << endl << "Swapped Solutions: " << swappedSolutions << endl;

	cout << endl << "Executions: " << executionCount << " (" << (100 * executionCount) / parameters.iterations << "%) " << endl << endl;

	for (vector<Heuristic*>::reverse_iterator it = heuristics->rbegin(); it != heuristics->rend(); it++) {
		(*it)->printStatistics('-', executionCount);
	}

	cout << COLOR_DEFAULT;
}

AteamsParameters Control::getParameters() {
	return parameters;
}

bool Control::setParameter(const char *parameter, const char *value) {
	return parameters.setParameter(parameter, value);
}

void Control::newHeuristic(Heuristic *alg) {
	if (alg != NULL) {
		heuristics->push_back(alg);

		Heuristic::heuristicsProbabilitySum += alg->getParameters().choiceProbability;
	}

	sort(heuristics->begin(), heuristics->end(), Heuristic::sortingComparator);
}

void Control::deleteHeuristic(Heuristic *alg) {
	if (alg != NULL) {
		remove(heuristics->begin(), heuristics->end(), alg);

		Heuristic::heuristicsProbabilitySum -= alg->getParameters().choiceProbability;
	}

	sort(heuristics->begin(), heuristics->end(), Heuristic::sortingComparator);
}

double Control::sumFitnessMaximize(set<Problem*, bool (*)(Problem*, Problem*)> *probs, int n) {
	set<Problem*>::const_iterator iter;
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
	set<Problem*>::const_iterator iter;
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

set<Problem*>::iterator Control::selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *probs, double fitTotal) {
	// Armazena o fitness total da populacao
	unsigned int sum = (unsigned int) fitTotal;
	// Um numero entre zero e "sum" e sorteado
	unsigned int randWheel = random(0, sum + 1);

	set<Problem*>::iterator iter;
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
	unsigned int randWheel = random(0, sum + 1);

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
	unsigned int randWheel = random(0, sum + 1);

	for (int i = 0; i < (int) heuristic->size(); i++) {
		sum -= heuristic->at(i)->getParameters().choiceProbability;
		if (sum <= randWheel) {
			return heuristic->at(i);
		}
	}

	return heuristic->at(0);
}

vector<Problem*>::iterator Control::selectRandom(vector<Problem*> *probs) {
	unsigned int randWheel = random(0, probs->size());

	vector<Problem*>::iterator iter = probs->begin();
	for (unsigned long i = 0; iter != probs->end() && i < randWheel; iter++, i++);

	return iter;
}

list<Problem*>::iterator Control::findSolution(list<Problem*> *vect, Problem *p) {
	list<Problem*>::iterator iter;

	for (iter = vect->begin(); iter != vect->end(); iter++)
		if (fnEqualSolution((*iter), p))
			return iter;

	return iter;
}

void Control::printProgress(HeuristicExecutionInfo *heuristic, pair<int, int> *insertion) {
	if (instance->showTextOverview) {
		string execNames;
		string color;

		execNames = Heuristic::getRunningHeuristics();

		if (insertion == NULL) {
			snprintf(Control::buffer, BUFFER_SIZE, ">>> {%.3ld} ALG: %s | ........................................... | QUEUE: %02d::[%s]", instance->executionCount, heuristic->heuristicInfo, runningThreads, execNames.c_str());
			color = COLOR_GREEN;
		} else {
			snprintf(Control::buffer, BUFFER_SIZE, "<<< {%.3ld} ALG: %s | FITNESS: %.6ld::%.6ld | CONTRIB: %.3d::%.3d | QUEUE: %02d::[%s]", instance->executionCount, heuristic->heuristicInfo, (long) Problem::best, (long) Problem::worst, insertion->first, insertion->second, runningThreads, execNames.c_str());
			color = COLOR_YELLOW;
		}

		cout << color << Control::buffer << COLOR_DEFAULT << endl << flush;
	} else {
		instance->executionProgressBar->update(instance->executionCount);
	}
}

void* Control::pthrExecution(void *iteration) {
	Control *ctrl = Control::instance;
	PrimitiveWrapper<unsigned int> *iterationAteams = (PrimitiveWrapper<unsigned int>*) iteration;

	uintptr_t inserted = 0;

	sem_wait(&semaphore);

	if (STATUS == EXECUTING) {
		inserted = ctrl->execute(iterationAteams->value);
	}

	sem_post(&semaphore);

	delete iterationAteams;

	pthread_return(inserted);
}

void* Control::pthrManagement(void *_) {
	Control *ctrl = Control::instance;
	time_t rawtime;

	while (STATUS == EXECUTING) {
		time(&rawtime);

		if (ctrl->parameters.maxExecutionTime != -1 && (int) difftime(rawtime, ctrl->startTime) > ctrl->parameters.maxExecutionTime)
			STATUS = EXECUTION_TIMEOUT;

		if (ctrl->parameters.attemptsWithoutImprovement != -1 && ctrl->lastImprovedIteration > ctrl->parameters.attemptsWithoutImprovement)
			STATUS = LACK_OF_IMPROVEMENT;

		if (ctrl->parameters.maxSolutions != -1 && Problem::totalNumInst > (unsigned long long) ctrl->parameters.maxSolutions)
			STATUS = TOO_MANY_SOLUTIONS;

		if ((ctrl->parameters.bestKnownFitness != -1 ? Problem::improvement(ctrl->parameters.bestKnownFitness, Problem::best) : -1) >= 0)
			STATUS = RESULT_ACHIEVED;

		sleep_ms(THREAD_MANAGEMENT_UPDATE_INTERVAL);
	}

	pthread_return(NULL);
}

inline string Heuristic::getHeuristicNames(list<HeuristicExecutionInfo*> *heuristicsList) {
	string executedNames;

	for (list<HeuristicExecutionInfo*>::const_iterator it = heuristicsList->cbegin(); it != heuristicsList->cend(); it++) {
		executedNames.append((*it)->heuristicInfo).push_back(' ');
	}
	if (executedNames.size() > 0) {
		executedNames.pop_back();
	}

	return executedNames;
}

Control *Control::instance = NULL;

int Control::runningThreads = 0;

char Control::buffer[BUFFER_SIZE];

int *Control::argc = NULL;
char **Control::argv = NULL;

int Heuristic::heuristicsProbabilitySum = 0;
list<HeuristicExecutionInfo*> *Heuristic::runningHeuristics = new list<HeuristicExecutionInfo*>;
list<HeuristicExecutionInfo*> *Heuristic::executedHeuristics = new list<HeuristicExecutionInfo*>;
