#include "Control.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <future>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "Constants.hpp"
#include "ProgressBar.hpp"
#include "Semaphore.hpp"

using namespace std;
using namespace chrono;
using namespace this_thread;

mutex mutex_population;			// Mutex que protege a populacao principal
mutex mutex_execution;			// Mutex que protege o executor de algoritimos
mutex mutex_counter;			// Mutex que protege os contadores de novas solucoes
mutex mutex_info;				// Mutex que protege a impressao das informacoes da execucao

semaphore semaphore_executor;	// Semaforo que controla o acesso dos algoritmos ao processador

Control* Control::getInstance(int argc, char **argv) {
	if (instance == NULL) {
		instance = new Control(argc, argv);
	} else {
		terminate();

		instance = getInstance(argc, argv);
	}

	return instance;
}

ExecutionInfo* Control::terminate() {
	ExecutionInfo *executionInfo = instance->getExecutionInfo();

	delete instance;
	instance = NULL;

	if (Problem::numInst != 0) {
		throw string("Solutions Leaked: ") + to_string(Problem::numInst);
	}

	Problem::deallocateMemory();

	return executionInfo;
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

int Control::findPosArgv(char **in, int num, char *key) {
	for (int i = 0; i < num; i++) {
		if (!strcmp(in[i], key))
			return i + 1;
	}

	return -1;
}

Control::Control(int argc, char **argv) {
	Heuristic::allocateMemory();

	this->argc = &argc;
	this->argv = argv;

	this->solutions = new set<Problem*, bool (*)(Problem*, Problem*)>(fnSortSolution);

	this->heuristicsProbabilitySum = 0;
	this->heuristics = new vector<Heuristic*>();

	this->readMainCMDParameters();
	this->readXMLFileParameters();
	this->readExtraCMDParameters();

	this->parameters.sanitizeParameters();

	this->loadingProgressBar = new ProgressBar("LOADING: ");
	this->executionProgressBar = new ProgressBar("EXECUTING: ");

	this->graphicalOverview = new GraphicalOverview(this);

	this->iterationsWithoutImprovement = 0;
	this->executionCount = 0;
	this->heuristicsSolutionsCount = 0;
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
	PopulationImprovement *populationImprovement = NULL;
	vector<Problem*> *newSoluctions = NULL;

	HeuristicExecutionInfo *info = NULL;
	Heuristic *algorithm = NULL;

	algorithm = Heuristic::selectOpportunisticHeuristic(heuristics, heuristicsProbabilitySum);
	info = new HeuristicExecutionInfo(algorithm, executionId, get_id());

	{
		scoped_lock<decltype(mutex_info)> lock_info_start(mutex_info);

		runningThreads++;

		Heuristic::heuristicStarted(info);

		Control::printProgress(info);
	}

	newSoluctions = algorithm->start(solutions, info);

	{
		scoped_lock<decltype(mutex_population)> lock_population(mutex_population);

		populationImprovement = insertNewSolutions(newSoluctions, true);

		if (populationImprovement->getImprovementOnBestSolution() > 0) {
			iterationsWithoutImprovement = 0;
		} else {
			iterationsWithoutImprovement++;
		}

		delete populationImprovement;
	}

	newSoluctions->clear();
	delete newSoluctions;

	{
		scoped_lock<decltype(mutex_info)> lock_info_end(mutex_info);

		executionCount++;

		runningThreads--;

		Heuristic::heuristicFinished(info);

		Control::printProgress(info);
	}

	return info;
}

PopulationImprovement* Control::insertNewSolutions(vector<Problem*> *newSolutions, bool autoTrim) {
	PopulationImprovement *populationImprovement = new PopulationImprovement();

	populationImprovement->setOldFitness(Problem::best, Problem::worst);

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

	populationImprovement->setNewFitness(Problem::best, Problem::worst);

	return populationImprovement;
}

void Control::trimSolutions() {
	int trimPosition = min((int) solutions->size(), max(parameters.maxPopulationSize, parameters.populationSize));
	set<Problem*>::iterator lastViableSolution = solutions->begin();
	advance(lastViableSolution, trimPosition);

	while (lastViableSolution != solutions->end()) {
		delete *lastViableSolution;

		lastViableSolution = solutions->erase(lastViableSolution);
	}

	int size = solutions->size();

	Problem::best = (*solutions->begin())->getFitness();
	Problem::worst = (*solutions->rbegin())->getFitness();
}

void Control::generateInitialPopulation() {
	cout << endl;

	loadingProgressBar->init(parameters.populationSize);

	if (savedPopulation != NULL) {
		for (list<Problem*>::iterator iter = savedPopulation->begin(); iter != savedPopulation->end(); iter++) {
			if ((int) solutions->size() >= parameters.populationSize || !solutions->insert(*iter).second) {
				delete *iter;
			}
		}
	}

	loadingProgressBar->update(solutions->size());

	unsigned long int limit = pow(10 * parameters.populationSize, 2), failedAttempts = 0;
	while ((int) solutions->size() < parameters.populationSize && failedAttempts < limit) {
		Problem *soluction = Problem::randomSolution();

		if (soluction->getFitness() == -1 || !solutions->insert(soluction).second) {
			failedAttempts++;

			delete soluction;
		}

		loadingProgressBar->update(solutions->size());
	}

	loadingProgressBar->end();

	if (!solutions->empty()) {
		Problem::best = (*solutions->begin())->getFitness();
		Problem::worst = (*solutions->rbegin())->getFitness();
	}

	return;
}

inline void Control::readMainCMDParameters() {
	int p = -1;

	Control::buffer[0] = '\0';

	if ((p = Control::findPosArgv(argv, *argc, (char*) "-p")) != -1) {
		strcpy(inputParameters, argv[p]);

		cout << COLOR_GREEN << "Parameters File: " << inputParameters << COLOR_DEFAULT << endl;
	} else {
		memset(inputParameters, 0, sizeof inputParameters);

		cout << COLOR_RED << "Parameters File: " << "---" << COLOR_DEFAULT << endl;

		strcat(Control::buffer, "Parameters File Cannot Be Empty! ");
	}

	if ((p = Control::findPosArgv(argv, *argc, (char*) "-i")) != -1) {
		strcpy(inputDataFile, argv[p]);

		cout << COLOR_GREEN << "Data File: " << inputDataFile << COLOR_DEFAULT << endl;
	} else {
		memset(inputDataFile, 0, sizeof inputDataFile);

		cout << COLOR_RED << "Data File: " << "---" << COLOR_DEFAULT << endl;

		strcat(Control::buffer, "Data File Cannot Be Empty! ");
	}

	if ((p = Control::findPosArgv(argv, *argc, (char*) "-r")) != -1) {
		strcpy(outputResultFile, argv[p]);

		cout << COLOR_GREEN << "Result File: " << outputResultFile << COLOR_DEFAULT << endl;
	} else {
		memset(outputResultFile, 0, sizeof outputResultFile);

		cout << COLOR_YELLOW << "Result File: " << "---" << COLOR_DEFAULT << endl;
	}

	if ((p = Control::findPosArgv(argv, *argc, (char*) "-t")) != -1) {
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

	setPrintFullSolution(Control::findPosArgv(argv, *argc, (char*) "-s") != -1);

	setGraphicStatusInfoScreen(Control::findPosArgv(argv, *argc, (char*) "-g") != -1);
	setCMDStatusInfoScreen(Control::findPosArgv(argv, *argc, (char*) "-c") != -1, Control::findPosArgv(argv, *argc, (char*) "-cc") != -1);
}

inline void Control::readExtraCMDParameters() {
	int p = -1;

	for (map<string, void*>::const_iterator param = parameters.keys.begin(); param != parameters.keys.end(); param++) {
		string cmdParameter = COMMAND_LINE_PARAMETER_SUFFIX + param->first;

		if ((p = Control::findPosArgv(argv, *argc, const_cast<char*>(cmdParameter.c_str()))) != -1) {
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

inline void Control::setGraphicStatusInfoScreen(bool showGraphicalOverview) {
	this->showGraphicalOverview = showGraphicalOverview;
}

inline void Control::setCMDStatusInfoScreen(bool showTextOverview, bool showQueueTextOverview) {
	this->showTextOverview = showTextOverview || showQueueTextOverview;
	this->showQueueTextOverview = showQueueTextOverview;
}

void Control::init() {
	semaphore_executor.setup(parameters.numThreads);

	if (heuristics->empty()) {
		throw string("No Heuristics Defined!");
	}

	/* Leitura dos dados passados por arquivos */
	Problem::readProblemFromFile(getInputDataFile());

	/* Le memoria prinipal do disco, se especificado */
	savedPopulation = Problem::readPopulationFromLog(getOutputLogFile());

	generateInitialPopulation();

	if (solutions->empty()) {
		throw string("No Initial Solution Found!");
	}

	cout << endl << COLOR_CYAN;

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

	delete savedPopulation;
	savedPopulation = NULL;

	list<Problem*> *finalSolutions = getSolutions();
	ExecutionInfo *executionInfo = getExecutionInfo();

	/* Escreve solucao em arquivo no disco */
	Problem::writeResultInFile(getInputDataFile(), getInputParameters(), getOutputResultFile(), executionInfo);

	/* Escreve memoria principal no disco */
	Problem::writeCurrentPopulationInLog(getOutputLogFile(), finalSolutions);

	delete executionInfo;
	delete finalSolutions;

	/* Testa a memoria principal por solucoes repetidas ou fora de ordem */
	for (set<Problem*>::const_iterator iter1 = solutions->begin(); iter1 != solutions->end(); iter1++) {
		for (set<Problem*>::const_iterator iter2 = iter1; iter2 != solutions->end(); iter2++) {
			if ((iter1 != iter2) && (fnEqualSolution(*iter1, *iter2) || fnSortSolution(*iter2, *iter1))) {
				throw "Incorrect Main Memory!";
			}
		}
	}

	semaphore_executor.setup(0);
}

void Control::run() {
	startTime = steady_clock::now();

	list<HeuristicExecutionInfo*> returnedHeuristics;

	STATUS = EXECUTING;

	if (showGraphicalOverview) {
		graphicalOverview->run();
	}

	if (!showTextOverview) {
		executionProgressBar->init(parameters.iterations);
	} else {
		cout << endl << flush;
	}

	future<TerminationInfo> management = async(launch::async, Control::threadManagement);

#if JOB_MANAGER
	queue<unsigned int> ids;
	for (int executionId = 1; executionId <= parameters.iterations; executionId++) {
		ids.push(executionId);
	}

	vector<future<list<HeuristicExecutionInfo*>>> executions;
	for (int job = 0; job < parameters.numThreads; job++) {
		executions.push_back(async(launch::async, Control::threadExecutions, &ids));
	}

	for (vector<future<list<HeuristicExecutionInfo*>>>::iterator futureIterator = executions.begin(); futureIterator != executions.end(); futureIterator++) {
		list<HeuristicExecutionInfo*> infos = futureIterator->get();

		returnedHeuristics.merge(infos);
	}
#else
	vector<future<HeuristicExecutionInfo*>> executions;
	for (int executionId = 1; executionId <= parameters.iterations; executionId++) {
		executions.push_back(async(launch::async, Control::threadExecution, executionId));
	}

	for (vector<future<HeuristicExecutionInfo*>>::iterator futureIterator = executions.begin(); futureIterator != executions.end(); futureIterator++) {
		HeuristicExecutionInfo *info = futureIterator->get();

		if (info != NULL) {
			returnedHeuristics.push_back(info);
		}
	}
#endif

	STATUS = management.get();

	if (!showTextOverview) {
		executionProgressBar->end();
	} else {
		cout << flush;
	}

	heuristicsSolutionsCount = HeuristicExecutionInfo::getContributionSum(returnedHeuristics);

	endTime = steady_clock::now();
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

ExecutionInfo* Control::getExecutionInfo() {
	return new ExecutionInfo(startTime, endTime, executionCount, heuristicsSolutionsCount);
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
	cout << endl << "Returned Solutions: " << heuristicsSolutionsCount << endl;

	int executionPercentage = parameters.iterations != 0 ? (100 * executionCount) / parameters.iterations : 100;

	cout << endl << "Executions: " << executionCount << " (" << executionPercentage << "%) " << endl << endl;

	for (vector<Heuristic*>::const_reverse_iterator it = heuristics->crbegin(); it != heuristics->crend(); it++) {
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

		heuristicsProbabilitySum += alg->getParameters().choiceProbability;
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

			heuristicsProbabilitySum -= alg->getParameters().choiceProbability;
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

	heuristicsProbabilitySum = 0;

	heuristics->clear();
}

void Control::printProgress(HeuristicExecutionInfo *info) {
	if (instance->showTextOverview) {
		string queue;
		string color;

		if (instance->showQueueTextOverview) {
			queue = "[" + Heuristic::getRunningHeuristics() + "]";
		}

		Control::buffer[0] = '\0';

		if (!info->isFinished()) {
			snprintf(Control::buffer, BUFFER_SIZE, ">>> {%.4d} ALG: %s | ....................... | QUEUE: %02d %s", instance->executionCount, info->heuristicInfo, runningThreads, queue.c_str());
			color = COLOR_YELLOW;
		} else {
			snprintf(Control::buffer, BUFFER_SIZE, "<<< {%.4d} ALG: %s | FITNESS: %.6ld::%.6ld | QUEUE: %02d %s", instance->executionCount, info->heuristicInfo, (long) Problem::best, (long) Problem::worst, runningThreads, queue.c_str());
			color = COLOR_GREEN;
		}

		cout << color << Control::buffer << COLOR_DEFAULT << endl << flush;

		Control::buffer[0] = '\0';
	} else {
		instance->executionProgressBar->update(instance->executionCount);
	}
}

list<HeuristicExecutionInfo*> Control::threadExecutions(queue<unsigned int> *ids) {
	Control *ctrl = Control::instance;

	list<HeuristicExecutionInfo*> infos;

	HeuristicExecutionInfo *inserted = NULL;

	while (STATUS == EXECUTING) {
		int executionId = -1;

		{
			scoped_lock<decltype(mutex_execution)> lock(mutex_execution);

			if (!ids->empty()) {
				executionId = ids->front();
				ids->pop();
			} else {
				break;
			}
		}

		inserted = Control::threadExecution(executionId);

		infos.push_back(inserted);
	}

	return infos;
}

HeuristicExecutionInfo* Control::threadExecution(unsigned int executionId) {
	Control *ctrl = Control::instance;

	HeuristicExecutionInfo *inserted = NULL;

	semaphore_executor.wait();

	if (STATUS == EXECUTING) {
		inserted = ctrl->execute(executionId);
	}

	semaphore_executor.notify();

	return inserted;
}

TerminationInfo Control::threadManagement() {
	Control *ctrl = Control::instance;

	while (STATUS == EXECUTING) {
		sleep_ms(THREAD_MANAGEMENT_UPDATE_INTERVAL);

		if (ctrl->executionCount == ctrl->parameters.iterations && (int) Heuristic::allHeuristics->size() == ctrl->parameters.iterations) {
			STATUS = FINISHED_NORMALLY;

			break;
		}

		if (ctrl->parameters.maxExecutionTime != -1 && duration_cast<seconds>(steady_clock::now() - ctrl->startTime).count() > ctrl->parameters.maxExecutionTime) {
			STATUS = EXECUTION_TIMEOUT;

			break;
		}

		if (ctrl->parameters.attemptsWithoutImprovement != -1 && ctrl->iterationsWithoutImprovement > ctrl->parameters.attemptsWithoutImprovement) {
			STATUS = LACK_OF_IMPROVEMENT;

			break;
		}

		if (ctrl->parameters.maxSolutions != -1 && Problem::totalNumInst > (unsigned long long) ctrl->parameters.maxSolutions) {
			STATUS = TOO_MANY_SOLUTIONS;

			break;
		}

		if ((ctrl->parameters.bestKnownFitness != -1 ? Problem::improvement(ctrl->parameters.bestKnownFitness, Problem::best) : -1) >= 0) {
			STATUS = RESULT_ACHIEVED;

			break;
		}
	}

	return STATUS;
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

list<HeuristicExecutionInfo*> *Heuristic::allHeuristics = NULL;
list<HeuristicExecutionInfo*> *Heuristic::runningHeuristics = NULL;
