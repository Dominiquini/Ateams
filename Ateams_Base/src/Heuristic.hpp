#include "Problem.hpp"

using namespace std;

#ifndef _HEURISTIC_
#define _HEURISTIC_

#define MAX_ATTEMPTS 50

#define HEURISTIC_NAME_MAX_LENGTH 16
#define HEURISTIC_INFO_MAX_LENGTH 512

extern pthread_mutex_t mutex_pop;

extern volatile TerminationInfo STATUS;

class HeuristicExecutionInfo;

struct HeuristicParameters {
	map<string, void*> keys;

	string name;

	int choiceProbability = 0;
	int choicePolicy = 0;

	bool setParameter(const char *parameter, const char *value) {
		int read = EOF;

		if (strcasecmp(parameter, "name") == 0) {
			name = string(value);

			read = name.size();
		} else {
			for (map<string, void*>::const_iterator param = keys.begin(); param != keys.end(); param++) {
				if (strcasecmp(parameter, param->first.c_str()) == 0) {
					if(strchr(value, '.') != NULL)
						read = sscanf(value, "%f", (float*) param->second);
					else
						read = sscanf(value, "%d", (int*) param->second);
				}
			}
		}

		return read != EOF;
	}

	bool isEqual(HeuristicParameters other) {
		return name.compare(other.name) == 0;
	}
};

class Heuristic {
public:

	static int heuristicsProbabilitySum;							// Soma das probabilidades de todos os algoritimos

	static list<HeuristicExecutionInfo*> *allHeuristics;			// Algoritmos executados ate o momento
	static list<HeuristicExecutionInfo*> *runningHeuristics;		// Algoritmos em execucao no momento

	static inline void allocateMemory();

	static inline void deallocateMemory();

	static bool sortingComparator(Heuristic *h1, Heuristic *h2) {
		return h1->getParameters().choiceProbability < h2->getParameters().choiceProbability;
	}

	static bool equalityComparator(Heuristic *h1, Heuristic *h2) {
		return h1->getParameters().isEqual(h2->getParameters());
	}

	static inline string getExecutedHeuristics() {
		return getHeuristicNames(allHeuristics);
	}

	static inline string getRunningHeuristics() {
		return getHeuristicNames(runningHeuristics);
	}

	int executionCounter;

	Heuristic() {
		executionCounter = 0;
	}

	virtual ~Heuristic() {
		executionCounter = -1;
	}

	virtual HeuristicParameters getParameters() = 0;

	virtual bool setParameter(const char *parameter, const char *value) = 0;

	virtual void printStatistics(char bullet, int total) {
		if (total == 0) {
			printf(" %c %s |%% %03d %%| -> %03d (%% ------ %%)\n", bullet, getParameters().name.c_str(), getParameters().choiceProbability, executionCounter);
		} else {
			printf(" %c %s |%% %03d %%| -> %03d (%% %06.2f %%)\n", bullet, getParameters().name.c_str(), getParameters().choiceProbability, executionCounter, (100.0f * executionCounter) / total);
		}
	}

	virtual set<Problem*>::const_iterator selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) = 0;

	virtual vector<Problem*>* start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicExecutionInfo *listener) = 0;

	virtual void markSolutions(vector<Problem*> *solutions) = 0;

private:

	static string getHeuristicNames(list<HeuristicExecutionInfo*> *heuristicsList);
};

class HeuristicExecutionInfo {
public:

	Heuristic *heuristic;

	unsigned int executionId;
	uintptr_t threadId;

	char heuristicInfo[HEURISTIC_NAME_MAX_LENGTH];

	double bestInitialFitness, bestActualFitness;

	double status;

	char execInfo[HEURISTIC_INFO_MAX_LENGTH];

	int newSolutionsProduced;

	HeuristicExecutionInfo(Heuristic *heuristic, unsigned int executionId, uintptr_t threadId) {
		this->heuristic = heuristic;

		this->executionId = executionId;
		this->threadId = threadId;

		this->bestInitialFitness = 0;
		this->bestActualFitness = 0;

		this->status = -1.0;

		this->newSolutionsProduced = 0;

		configName();
	}

	~HeuristicExecutionInfo() {
	}

	inline void setupInfo(const char *format, ...) {
		va_list args;
		va_start(args, format);
		vsnprintf(this->execInfo, HEURISTIC_INFO_MAX_LENGTH, format, args);
		va_end(args);
	}

	inline bool isStarted() {
		return status != -1;
	}

	inline bool isFinished() {
		return status == 100.0;
	}

private:

	inline void configName() {
		if (heuristic != NULL) {
			snprintf(heuristicInfo, HEURISTIC_NAME_MAX_LENGTH, "%s(%04d)", heuristic->getParameters().name.c_str(), executionId);
		}
	}
};

inline void Heuristic::allocateMemory() {
	heuristicsProbabilitySum = 0;

	allHeuristics = new list<HeuristicExecutionInfo*>;
	runningHeuristics = new list<HeuristicExecutionInfo*>;
}

inline void Heuristic::deallocateMemory() {
	heuristicsProbabilitySum = 0;

	for (list<HeuristicExecutionInfo*>::iterator it = allHeuristics->begin(); it != allHeuristics->end(); it++) {
		delete *it;
	}

	allHeuristics->clear();
	delete allHeuristics;
	allHeuristics = NULL;

	runningHeuristics->clear();
	delete runningHeuristics;
	runningHeuristics = NULL;
}

#endif
