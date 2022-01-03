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

class Heuristic {
public:

	static int heuristicsProbabilitySum;								// Soma das probabilidades de todos os algoritimos

	static list<HeuristicExecutionInfo*> *executedHeuristics;			// Algoritmos executados ate o momento
	static list<HeuristicExecutionInfo*> *runningHeuristics;			// Algoritmos em execucao no momento

	static inline bool comparator(Heuristic *h1, Heuristic *h2) {
		return h1->choiceProbability < h2->choiceProbability;
	}

	static inline string getExecutedHeuristics() {
		return getHeuristicNames(executedHeuristics);
	}

	static inline string getRunningHeuristics() {
		return getHeuristicNames(runningHeuristics);
	}

	string name;
	int executionCounter = 0;
	int choiceProbability = 0, choicePolicy = 0;

	Heuristic(string heuristicName) {
		name = heuristicName;
	}

	virtual ~Heuristic() {
	}

	virtual void printStatistics(char bullet, int total) {
		printf(" %c %s |%% %03d %%| -> %03d (%% %06.2f %%)\n", bullet, name.c_str(), choiceProbability, executionCounter, (100.0f * executionCounter) / total);
	}

	virtual bool setParameter(const char *parameter, const char *value) {
		if (strcasecmp(parameter, "name") != 0)
			return false;

		name = string(value);

		return true;
	}

	virtual set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) = 0;

	virtual vector<Problem*>* start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicExecutionInfo *listener) = 0;

	virtual void markSolutions(vector<Problem*> *solutions) = 0;

private:

	static string getHeuristicNames(list<HeuristicExecutionInfo*> *heuristicsList);
};

class HeuristicExecutionInfo {
public:

	Heuristic *heuristic;
	thread::id threadId;
	int executionId;

	char heuristicInfo[HEURISTIC_NAME_MAX_LENGTH];

	double bestInitialFitness, bestActualFitness;

	double status;

	char execInfo[HEURISTIC_INFO_MAX_LENGTH];

	HeuristicExecutionInfo(Heuristic *heuristic, int executionId, thread::id threadId) {
		this->heuristic = heuristic;
		this->executionId = executionId;
		this->threadId = threadId;

		this->bestInitialFitness = 0;
		this->bestActualFitness = 0;

		this->status = -1;

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

private:

	inline void configName() {
		snprintf(heuristicInfo, HEURISTIC_NAME_MAX_LENGTH, "%s(%04d)", heuristic->name.c_str(), executionId);
	}
};

#endif
