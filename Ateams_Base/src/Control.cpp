#include "Control.hpp"

using namespace std;

pthread_mutexattr_t mutex_attr;

pthread_mutex_t mutex_pop = PTHREAD_MUTEX_INITIALIZER;	// Mutex que protege a populacao principal
pthread_mutex_t mutex_cont = PTHREAD_MUTEX_INITIALIZER;	// Mutex que protege as variaveis de criacao de novas solucoes
pthread_mutex_t mutex_info = PTHREAD_MUTEX_INITIALIZER;	// Mutex que protege a impressao das informacoes da execucao
pthread_mutex_t mutex_exec = PTHREAD_MUTEX_INITIALIZER;	// Mutex que protege as informacoes de execucao

sem_t semaphore;			// Semaforo que controla o acesso dos algoritmos ao processador

Control* Control::getInstance(int argc, char **argv) {
	if (instance == NULL) {
		instance = new Control(argc, argv);
	} else {
		terminate();

		instance = getInstance(argc, argv);
	}

	return instance;
}

void Control::terminate() {
	delete instance;
	instance = NULL;

	if (Problem::numInst != 0) {
		throw string("Solutions Leaked: ") + to_string(Problem::numInst);
	}

	Problem::deallocateMemory();
}

Heuristic* Control::instantiateHeuristic(char *name) {
	if (strcasecmp(name, SIMULATED_ANNEALING_NAME) == 0)
		return new SimulatedAnnealing();

	if (strcasecmp(name, GENETIC_ALGORITHM_NAME) == 0)
		return new GeneticAlgorithm();

	if (strcasecmp(name, TABU_SEARCH_NAME) == 0)
		return new TabuSearch();

	return NULL;
}

Control::Control(int argc, char **argv) {
	Heuristic::allocateMemory();

	this->argc = &argc;
	this->argv = argv;

	this->solutions = new set<Problem*, bool (*)(Problem*, Problem*)>(fnSortSolution);

	this->heuristics = new vector<Heuristic*>();

	this->readMainCMDParameters();
	this->readXMLFileParameters();
	this->readExtraCMDParameters();

	this->loadingProgressBar = new ProgressBar("LOADING: ");
	this->executionProgressBar = new ProgressBar("EXECUTING: ");

	this->graphicalOverview = new GraphicalOverview(this->showGraphicalOverview, this->argc, this->argv);

	this->startTime = this->endTime = 0;
	this->lastImprovedIteration = 0;
	this->executionCount = 0;
	this->newSolutionsCount = 0;
}

Control::~Control() {
	for (set<Problem*>::iterator iter = solutions->begin(); iter != solutions->end(); iter++) {
		delete *iter;
	}

	solutions->clear();
	delete solutions;

	for (vector<Heuristic*>::reverse_iterator it = heuristics->rbegin(); it != heuristics->rend(); it++) {
		delete *it;
	}

	heuristics->clear();
	delete heuristics;

	delete loadingProgressBar;
	delete executionProgressBar;

	delete graphicalOverview;

	Heuristic::deallocateMemory();
}

HeuristicExecutionInfo* Control::execute(unsigned int executionId) {
	vector<Problem*> *newSoluctions = NULL;
	HeuristicExecutionInfo *info = NULL;
	Heuristic *algorithm = NULL;

	algorithm = selectRouletteWheel(heuristics, Heuristic::heuristicsProbabilitySum);
	info = new HeuristicExecutionInfo(algorithm, executionId, pthread_self());

	pthread_mutex_lock(&mutex_info);
	{
		runningThreads++;

		Heuristic::allHeuristics->push_back(info);
		Heuristic::runningHeuristics->push_back(info);

		Control::printProgress(info);
	}
	pthread_mutex_unlock(&mutex_info);

	newSoluctions = algorithm->start(solutions, info);

	pthread_mutex_lock(&mutex_pop);
	{
		executionCount++;

		double oldBest = Problem::best;

		insertNewSolutions(newSoluctions, true);

		double newBest = Problem::best;

		if (Problem::improvement(oldBest, newBest) > 0) {
			lastImprovedIteration = 0;
		} else {
			lastImprovedIteration++;
		}
	}
	pthread_mutex_unlock(&mutex_pop);

	newSoluctions->clear();
	delete newSoluctions;

	pthread_mutex_lock(&mutex_info);
	{
		runningThreads--;

		list<HeuristicExecutionInfo*>::iterator executed = find(Heuristic::runningHeuristics->begin(), Heuristic::runningHeuristics->end(), info);
		Heuristic::runningHeuristics->erase(executed);

		Control::printProgress(info);
	}
	pthread_mutex_unlock(&mutex_info);

	return info;
}

void Control::insertNewSolutions(vector<Problem*> *newSolutions, bool autoTrim) {
	for (vector<Problem*>::iterator it = newSolutions->begin(); it != newSolutions->end(); it++) {
		if (!solutions->insert(*it).second) {
			delete *it;
		}
	}

	if (autoTrim) {
		trimSolutions();
	}

	Problem::best = (*solutions->begin())->getFitness();
	Problem::worst = (*solutions->rbegin())->getFitness();
}

void Control::trimSolutions() {
	set<Problem*>::iterator lastViableSolution = solutions->begin();
	advance(lastViableSolution, max(parameters.maxPopulationSize, parameters.populationSize));

	while (lastViableSolution != solutions->end()) {
		delete *lastViableSolution;

		lastViableSolution = solutions->erase(lastViableSolution);
	}

	Problem::best = (*solutions->begin())->getFitness();
	Problem::worst = (*solutions->rbegin())->getFitness();
}

void Control::generatePopulation() {
	cout << endl;

	loadingProgressBar->init(parameters.populationSize);

	if (initialPopulation != NULL) {
		for (list<Problem*>::iterator iter = initialPopulation->begin(); iter != initialPopulation->end(); iter++) {
			if ((int) solutions->size() >= parameters.populationSize || !solutions->insert(*iter).second) {
				delete *iter;
			}
		}
	}

	loadingProgressBar->update(solutions->size());

	Problem *soluction = NULL;
	unsigned long int limit = pow(parameters.populationSize, 3), failedAttempts = 0;

	while ((int) solutions->size() < parameters.populationSize && failedAttempts < limit) {
		soluction = Problem::randomSolution();

		if (soluction->getFitness() == -1 || !solutions->insert(soluction).second) {
			failedAttempts++;

			delete soluction;
		}

		loadingProgressBar->update(solutions->size());
	}

	loadingProgressBar->end();

	cout << endl;

	if (!solutions->empty()) {
		Problem::best = (*solutions->begin())->getFitness();
		Problem::worst = (*solutions->rbegin())->getFitness();
	}

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

	Control::buffer[0] = '\0';

	if ((p = findPosArgv(argv, *argc, (char*) "-p")) != -1) {
		strcpy(inputParameters, argv[p]);

		cout << COLOR_GREEN << "Parameters File: " << inputParameters << COLOR_DEFAULT << endl;
	} else {
		memset(inputParameters, 0, sizeof inputParameters);

		cout << COLOR_RED << "Parameters File: " << "---" << COLOR_DEFAULT << endl;

		strcat(Control::buffer, "Parameters File Cannot Be Empty! ");
	}

	if ((p = findPosArgv(argv, *argc, (char*) "-i")) != -1) {
		strcpy(inputDataFile, argv[p]);

		cout << COLOR_GREEN << "Data File: " << inputDataFile << COLOR_DEFAULT << endl;
	} else {
		memset(inputDataFile, 0, sizeof inputDataFile);

		cout << COLOR_RED << "Data File: " << "---" << COLOR_DEFAULT << endl;

		strcat(Control::buffer, "Data File Cannot Be Empty! ");
	}

	if ((p = findPosArgv(argv, *argc, (char*) "-r")) != -1) {
		strcpy(outputResultFile, argv[p]);

		cout << COLOR_GREEN << "Result File: " << outputResultFile << COLOR_DEFAULT << endl;
	} else {
		memset(outputResultFile, 0, sizeof outputResultFile);

		cout << COLOR_YELLOW << "Result File: " << "---" << COLOR_DEFAULT << endl;
	}

	if ((p = findPosArgv(argv, *argc, (char*) "-t")) != -1) {
		strcpy(populationFile, argv[p]);

		cout << COLOR_GREEN << "Population File: " << populationFile << COLOR_DEFAULT << endl;
	} else {
		memset(populationFile, 0, sizeof populationFile);

		cout << COLOR_YELLOW << "Population File: " << "---" << COLOR_DEFAULT << endl;
	}

	if (strlen(Control::buffer) != 0) {
		throw string(Control::buffer);
	} else {
		Control::buffer[0] = '\0';
	}

	setPrintFullSolution(findPosArgv(argv, *argc, (char*) "-s") != -1);

	setCMDStatusInfoScreen(findPosArgv(argv, *argc, (char*) "-c") != -1);
	setGraphicStatusInfoScreen(findPosArgv(argv, *argc, (char*) "-g") != -1);
}

inline void Control::readExtraCMDParameters() {
	int p = -1;

	for (map<string, void*>::const_iterator param = parameters.keys.begin(); param != parameters.keys.end(); param++) {
		string cmdParameter = COMMAND_LINE_PARAMETER_SUFFIX + param->first;

		if ((p = findPosArgv(argv, *argc, const_cast<char*>(cmdParameter.c_str()))) != -1) {
			setParameter(param->first.c_str(), argv[p]);
		}
	}
}

inline void Control::readXMLFileParameters() {
	XMLParser *parser = new XMLParser(this);

	parser->parseXML(getInputParameters());

	delete parser;
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
	sem_init(&semaphore, 0, parameters.numThreads);

	/* Leitura dos dados passados por arquivos */
	Problem::readProblemFromFile(getInputDataFile());

	/* Le memoria prinipal do disco, se especificado */
	initialPopulation = Problem::readPopulationFromLog(getOutputLogFile());

	generatePopulation();

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

	delete initialPopulation;
	initialPopulation = NULL;

	list<Problem*> *finalSolutions = getSolutions();

	/* Escreve solucao em arquivo no disco */
	Problem::writeResultInFile(getInputDataFile(), getInputParameters(), getExecutionInfo(), getOutputResultFile());

	/* Escreve memoria principal no disco */
	Problem::writeCurrentPopulationInLog(getOutputLogFile(), finalSolutions);

	delete finalSolutions;

	/* Testa a memoria principal por solucoes repetidas ou fora de ordem */
	for (set<Problem*>::const_iterator iter1 = solutions->begin(); iter1 != solutions->end(); iter1++) {
		for (set<Problem*>::const_iterator iter2 = iter1; iter2 != solutions->end(); iter2++) {
			if ((iter1 != iter2) && (fnEqualSolution(*iter1, *iter2) || fnSortSolution(*iter2, *iter1))) {
				throw "Incorrect Main Memory!";
			}
		}
	}

	sem_destroy(&semaphore);
}

void Control::run() {
	STATUS = EXECUTING;

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

	if (pthread_create(&threadManagement, &attrJoinable, Control::pthrManagement, NULL) != 0) {
		throw "Thread Creation Error! (pthrManagement)";
	}

	for (int execAteams = 0; execAteams < parameters.iterations; execAteams++) {
		PrimitiveWrapper<unsigned int> *iteration = new PrimitiveWrapper<unsigned int>(execAteams + 1);

		if (pthread_create(&threads[execAteams], &attrJoinable, Control::pthrExecution, (void*) iteration) != 0) {
			throw "Thread Creation Error! (pthrExecution)";
		}
	}

	for (uintptr_t execAteams = 0, *inserted = NULL; execAteams < (uintptr_t) parameters.iterations; execAteams++) {
		pthread_join(threads[execAteams], (void**) &inserted);

		HeuristicExecutionInfo *info = (HeuristicExecutionInfo*) inserted;

		if (info != NULL) {
			newSolutionsCount += info->contribution;
		}
	}

	if (STATUS == EXECUTING) {
		STATUS = (executionCount == parameters.iterations && (int) Heuristic::allHeuristics->size() == parameters.iterations) ? FINISHED_NORMALLY : INCOMPLETE;
	}

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
	cout << endl << "Returned Solutions: " << newSolutionsCount << endl;

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

bool Control::insertHeuristic(Heuristic *alg, bool deleteIfNotInserted) {
	bool inserted = false;

	if (alg != NULL && alg->getParameters().choiceProbability >= 0) {
		inserted = true;

		heuristics->push_back(alg);

		Heuristic::heuristicsProbabilitySum += alg->getParameters().choiceProbability;
	}

	sort(heuristics->begin(), heuristics->end(), Heuristic::sortingComparator);

	if (!inserted && deleteIfNotInserted) {
		delete alg;
	}

	return inserted;
}

bool Control::removeHeuristic(Heuristic *alg, bool deleteIfRemoved) {
	bool removed = false;

	if (alg != NULL && alg->getParameters().choiceProbability >= 0) {
		vector<Heuristic*>::iterator it = find(heuristics->begin(), heuristics->end(), alg);

		if (it != heuristics->end()) {
			removed = true;

			heuristics->erase(it);

			Heuristic::heuristicsProbabilitySum -= alg->getParameters().choiceProbability;
		}
	}

	sort(heuristics->begin(), heuristics->end(), Heuristic::sortingComparator);

	if (removed && deleteIfRemoved) {
		delete alg;
	}

	return removed;
}

void Control::clearHeuristics(bool deleteHeuristics) {
	if (deleteHeuristics) {
		for (vector<Heuristic*>::iterator it = heuristics->begin(); it != heuristics->end(); it++) {
			delete *it;
		}
	}

	Heuristic::heuristicsProbabilitySum = 0;

	heuristics->clear();
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

set<Problem*>::const_iterator Control::selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *probs, double fitTotal) {
	// Armazena o fitness total da populacao
	unsigned int sum = (unsigned int) fitTotal;
	// Um numero entre zero e "sum" e sorteado
	unsigned int randWheel = randomNumber(0, sum);

	set<Problem*>::const_iterator iter;
	for (iter = probs->cbegin(); iter != probs->cend(); iter++) {
		sum -= (int) (*iter)->getFitnessMaximize();
		if (sum <= randWheel) {
			return iter;
		}
	}

	return (probs->begin());
}

vector<Problem*>::const_iterator Control::selectRouletteWheel(vector<Problem*> *probs, double fitTotal) {
	// Armazena o fitness total da populacao
	unsigned int sum = (unsigned int) fitTotal;
	// Um numero entre zero e "sum" e sorteado
	unsigned int randWheel = randomNumber(0, sum);

	vector<Problem*>::const_iterator iter;
	for (iter = probs->cbegin(); iter != probs->cend(); iter++) {
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
	unsigned int randWheel = randomNumber(0, sum);

	for (int i = 0; i < (int) heuristic->size(); i++) {
		sum -= heuristic->at(i)->getParameters().choiceProbability;
		if (sum <= randWheel) {
			return heuristic->at(i);
		}
	}

	return heuristic->at(0);
}

vector<Problem*>::iterator Control::selectRandom(vector<Problem*> *probs) {
	unsigned int randWheel = randomNumber(0, probs->size());

	vector<Problem*>::iterator iter = probs->begin();

	advance(iter, randWheel);

	return iter;
}

list<Problem*>::iterator Control::findSolution(list<Problem*> *vect, Problem *p) {
	list<Problem*>::iterator iter;

	for (iter = vect->begin(); iter != vect->end(); iter++)
		if (fnEqualSolution((*iter), p))
			return iter;

	return iter;
}

void Control::printProgress(HeuristicExecutionInfo *heuristic) {
	if (instance->showTextOverview) {
		string execNames;
		string color;

		execNames = Heuristic::getRunningHeuristics();

		Control::buffer[0] = '\0';

		if (!heuristic->isFinished()) {
			snprintf(Control::buffer, BUFFER_SIZE, ">>> {%.3ld} ALG: %s | ....................... | QUEUE: %02d::[%s]", instance->executionCount, heuristic->heuristicInfo, runningThreads, execNames.c_str());
			color = COLOR_GREEN;
		} else {
			snprintf(Control::buffer, BUFFER_SIZE, "<<< {%.3ld} ALG: %s | FITNESS: %.6ld::%.6ld | QUEUE: %02d::[%s]", instance->executionCount, heuristic->heuristicInfo, (long) Problem::best, (long) Problem::worst, runningThreads, execNames.c_str());
			color = COLOR_YELLOW;
		}

		cout << color << Control::buffer << COLOR_DEFAULT << endl << flush;

		Control::buffer[0] = '\0';
	} else {
		instance->executionProgressBar->update(instance->executionCount);
	}
}

void* Control::pthrExecution(void *iteration) {
	Control *ctrl = Control::instance;
	PrimitiveWrapper<unsigned int> *iterationAteams = (PrimitiveWrapper<unsigned int>*) iteration;

	HeuristicExecutionInfo *inserted = NULL;

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

int Heuristic::heuristicsProbabilitySum = 0;
list<HeuristicExecutionInfo*> *Heuristic::allHeuristics = NULL;
list<HeuristicExecutionInfo*> *Heuristic::runningHeuristics = NULL;
