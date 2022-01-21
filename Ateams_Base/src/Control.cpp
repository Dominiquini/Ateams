#include "Control.hpp"

using namespace std;
using namespace pugi;
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

Heuristic* Control::instantiateHeuristic(const char *name) {
	if (strcasecmp(name, SIMULATED_ANNEALING_TAG_NAME) == 0)
		return new SimulatedAnnealing();

	if (strcasecmp(name, GENETIC_ALGORITHM_TAG_NAME) == 0)
		return new GeneticAlgorithm();

	if (strcasecmp(name, TABU_SEARCH_TAG_NAME) == 0)
		return new TabuSearch();

	return NULL;
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

	this->graphicalOverview = GraphicalOverview::getInstance(this);

	this->executionCount = 0;
	this->iterationsWithoutImprovement = 0;

	this->heuristicsSolutionsCount = 0;
	this->heuristicsTotalExecutionTime = milliseconds::zero();
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

	GraphicalOverview::destroyInstance();

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
	int argumentPosition = -1;

	char error[CONTROL_MAX_ERROR_SIZE];
	memset(error, 0, sizeof error);

	{
		if ((argumentPosition = findCMDArgumentPosition("-p")) != -1) {
			strcpy(inputParameters, argv[argumentPosition]);

			cout << COLOR_GREEN << "Parameters File: " << inputParameters << COLOR_DEFAULT << endl;
		} else {
			memset(inputParameters, 0, sizeof inputParameters);

			cout << COLOR_RED << "Parameters File: " << "---" << COLOR_DEFAULT << endl;

			strcat(error, "Parameters File Cannot Be Empty! ");
		}

		if ((argumentPosition = findCMDArgumentPosition("-i")) != -1) {
			strcpy(inputDataFile, argv[argumentPosition]);

			cout << COLOR_GREEN << "Data File: " << inputDataFile << COLOR_DEFAULT << endl;
		} else {
			memset(inputDataFile, 0, sizeof inputDataFile);

			cout << COLOR_RED << "Data File: " << "---" << COLOR_DEFAULT << endl;

			strcat(error, "Data File Cannot Be Empty! ");
		}

		if ((argumentPosition = findCMDArgumentPosition("-r")) != -1) {
			strcpy(outputResultFile, argv[argumentPosition]);

			cout << COLOR_GREEN << "Result File: " << outputResultFile << COLOR_DEFAULT << endl;
		} else {
			memset(outputResultFile, 0, sizeof outputResultFile);

			cout << COLOR_YELLOW << "Result File: " << "---" << COLOR_DEFAULT << endl;
		}

		if ((argumentPosition = findCMDArgumentPosition("-o")) != -1) {
			strcpy(populationFile, argv[argumentPosition]);

			cout << COLOR_GREEN << "Population File: " << populationFile << COLOR_DEFAULT << endl;
		} else {
			memset(populationFile, 0, sizeof populationFile);

			cout << COLOR_YELLOW << "Population File: " << "---" << COLOR_DEFAULT << endl;
		}
	}

	if (strlen(error) != 0) {
		throw string(error);
	}

	{
		setPrintFullSolution(findCMDArgumentPosition("-s") != -1);

		setGraphicStatusInfoScreen(findCMDArgumentPosition("-g") != -1);

		setCMDStatusInfoScreen(findCMDArgumentPosition("-c") != -1, findCMDArgumentPosition("-cc") != -1, findCMDArgumentPosition("-ccc") != -1, findCMDArgumentPosition("-cccc") != -1, findCMDArgumentPosition("-ccccc") != -1);

		setSingleExecutionPerThread(findCMDArgumentPosition("-t") != -1);
	}
}

inline void Control::readExtraCMDParameters() {
	int argumentPosition = -1;

	for (map<string, void*>::const_iterator param = parameters.keys.begin(); param != parameters.keys.end(); param++) {
		string cmdParameter = "--" + param->first;

		if ((argumentPosition = findCMDArgumentPosition(cmdParameter)) != -1) {
			setParameter(param->first.c_str(), argv[argumentPosition]);
		}
	}
}

inline void Control::readXMLFileParameters() {
	clearHeuristics(true);

	xml_document parametersFile;

	xml_parse_result parseResult = parametersFile.load_file(getInputParameters());

	if (!parseResult) {
		throw string("Unable To Parse: ") + string(getInputParameters()) + string(" :: ") + string("(") + toProperCase(parseResult.description()) + string(")");
	}

	xml_node controllerNode = parametersFile.child(XML_TAG_CONTROLLER);

	if (controllerNode.empty()) {
		throw string("No Controller Defined!");
	}

	for (xml_attribute controllerAttr : controllerNode.attributes()) {
		auto parameter = controllerAttr.name();
		auto value = controllerAttr.value();

		if (!setParameter(parameter.data(), value.data())) {
			throw string("Invalid Controller Parameter: ").append(parameter);
		}
	}

	xml_node heuristicsNode = controllerNode.child(XML_TAG_HEURISTICS);

	if (heuristicsNode.empty()) {
		throw string("No Heuristics Defined!");
	}

	for (xml_node heuristicNode : heuristicsNode.children()) {
		auto heuristicName = heuristicNode.name();

		Heuristic *newHeuristic = Control::instantiateHeuristic(heuristicName.data());

		if (newHeuristic == NULL) {
			throw string("Invalid Heuristic Name: ").append(heuristicName);
		}

		for (xml_attribute heuristicAttr : heuristicNode.attributes()) {
			auto parameter = heuristicAttr.name();
			auto value = heuristicAttr.value();

			if (!newHeuristic->setParameter(parameter.data(), value.data())) {
				throw string("Invalid Heuristic Parameter: ").append(parameter).append(" (").append(newHeuristic->getParameters().name).append(") ");
			}
		}

		insertHeuristic(newHeuristic, true);
	}
}

inline void Control::setPrintFullSolution(bool fullPrint) {
	this->printFullSolution = fullPrint;
}

inline void Control::setGraphicStatusInfoScreen(bool showGraphicalOverview) {
	this->showGraphicalOverview = showGraphicalOverview;
}

inline void Control::setCMDStatusInfoScreen(bool showText, bool showFitness, bool showImprovement, bool showThread, bool showQueue) {
	this->showTextOverview = showText || showFitness || showImprovement || showThread || showQueue;
	this->showTextFitnessOverview = showFitness || showImprovement || showThread || showQueue;
	this->showTextImprovementOverview = showImprovement || showThread || showQueue;
	this->showTextThreadOverview = showThread || showQueue;
	this->showTextQueueOverview = showQueue;
}

inline void Control::setSingleExecutionPerThread(bool singleExecutionPerThread) {
	this->singleExecutionPerThread = singleExecutionPerThread;
}

inline int Control::findCMDArgumentPosition(string key) {
	for (int i = 0; i < *argc; i++) {
		if (!strcmp(argv[i], key.c_str()))
			return i + 1;
	}

	return -1;
}

void Control::start() {
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

	graphicalOverview->start();

	executionTime = milliseconds::zero();

	printPopulationInfo("Initial");
}

void Control::finish() {
	printPopulationInfo("Final");

	executionTime = duration_cast<milliseconds>(endTime - startTime);

	graphicalOverview->stop();

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
	list<HeuristicExecutionInfo*> returnedHeuristics;

	startTime = steady_clock::now();

	STATUS = EXECUTING;

	if (!showTextOverview) {
		executionProgressBar->init(parameters.iterations);
	} else {
		cout << endl << flush;
	}

	future<TerminationInfo> management = async(launch::async, Control::threadManagement);

	if (singleExecutionPerThread) {
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
	} else {
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
	}

	STATUS = management.get();

	if (!showTextOverview) {
		executionProgressBar->end();
	} else {
		cout << flush;
	}

	heuristicsSolutionsCount = HeuristicExecutionInfo::getContributionSum(returnedHeuristics);
	heuristicsTotalExecutionTime = HeuristicExecutionInfo::getTotalExecutionTime(returnedHeuristics);

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
	return new ExecutionInfo(executionCount, executionTime, heuristicsSolutionsCount, heuristicsTotalExecutionTime);
}

void Control::printPopulationInfo(string status) {
	cout << endl << COLOR_CYAN;

	cout << endl << status << " " << "Population Size: : " << solutions->size() << endl;
	cout << endl << "Worst" << " " << status << " " << "Solution: " << Problem::worst << endl;
	cout << endl << "Best" << " " << status << " " << "Solution: " << Problem::best << endl;

	cout << endl << COLOR_DEFAULT;
}

void Control::printSolutionInfo() {
	Problem *solution = getSolution(0);

	cout << endl << COLOR_MAGENTA;

	cout << "Best Solution: " << solution->getFitness() << endl << endl;

	solution->print(printFullSolution);

	cout << endl << COLOR_DEFAULT;
}

void Control::printExecutionInfo() {
	cout << endl << COLOR_BLUE;

	cout << "Explored Solutions: " << Problem::totalNumInst << endl << endl;
	cout << "Returned Solutions: " << heuristicsSolutionsCount << endl << endl;

	int executionPercentage = parameters.iterations != 0 ? (100 * executionCount) / parameters.iterations : 100;
	int executionTime = duration_cast<seconds>(heuristicsTotalExecutionTime).count();

	cout << "Executions: " << executionCount << " (" << executionPercentage << "%) " << "||" << " " << executionTime << "s" << endl << endl;

	for (vector<Heuristic*>::const_reverse_iterator it = heuristics->crbegin(); it != heuristics->crend(); it++) {
		(*it)->printStatistics(executionCount);
	}

	cout << endl << COLOR_DEFAULT;
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
		string indicator = info->isFinished() ? "<<<" : ">>>";
		string color = info->isFinished() ? COLOR_GREEN : COLOR_YELLOW;

		ostringstream progress;

		progress << "IT: " << stream_formatter(4, '0') << instance->executionCount;

		progress << " || ALGORITHM: " << info->heuristicInfo;

		if (instance->showTextFitnessOverview) {
			progress << " || FITNESS: " << stream_formatter(6, '0') << Problem::best << "::" << stream_formatter(6, '0') << Problem::worst;
		}

		if (instance->showTextImprovementOverview) {
			progress << " || BOOST: " << stream_formatter(6, '0') << info->bestInitialFitness << "::" << stream_formatter(6, '0') << info->bestActualFitness << "::" << stream_formatter(3, '0') << info->contribution;
		}

		if (instance->showTextThreadOverview) {
			progress << " || THREAD: " << stream_formatter(3, '0') << info->threadId;
		}

		if (instance->showTextQueueOverview) {
			progress << " || QUEUE: " << stream_formatter(3, '0') << runningThreads << "::" << '[' << Heuristic::getRunningHeuristics() << ']';
		}

		cout << color << indicator << ' ' << progress.str() << ' ' << indicator << COLOR_DEFAULT << endl << flush;
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
		sleep_ms(CONTROL_MANAGEMENT_UPDATE_INTERVAL);

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

list<HeuristicExecutionInfo*> *Heuristic::allHeuristics = NULL;
list<HeuristicExecutionInfo*> *Heuristic::runningHeuristics = NULL;
