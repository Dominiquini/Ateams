#include "Problem.hpp"

using namespace std;

#ifndef _HEURISTIC_
#define _HEURISTIC_

#define MAX_ATTEMPTS 50

extern pthread_mutex_t mutex_pop;

extern volatile TerminationInfo STATUS;

class HeuristicListener;

class Heuristic {
public:

	static int heuristicsProbabilitySum;								// Soma das probabilidades de todos os algoritimos

	static list<HeuristicListener*> *executedHeuristics;				// Algoritmos executados ate o momento
	static list<HeuristicListener*> *runningHeuristics;					// Algoritmos em execucao no momento

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

	virtual vector<Problem*>* start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicListener *listener) = 0;

	virtual void markSolutions(vector<Problem*> *solutions) = 0;

private:

	static string getHeuristicNames(list<HeuristicListener*> *heuristicsList);
};

class HeuristicListener {
public:

	Heuristic *heuristic;
	int id;

	char heuristicInfo[16];

	volatile double bestInitialFitness, bestActualFitness;
	volatile double status;
	char execInfo[512];

	HeuristicListener(Heuristic *heuristic, int threadId) {
		this->heuristic = heuristic;
		this->id = threadId;

		this->bestInitialFitness = 0;
		this->bestActualFitness = 0;
		this->status = -1;

		sprintf(heuristicInfo, "%s(%.3d)", heuristic->name.c_str(), threadId);
	}

	~HeuristicListener() {
	}

	inline void setuupInfo(const char *format, ...) {
		va_list args;
		va_start(args, format);
		vsprintf(this->execInfo, format, args);
		va_end(args);
	}
};

#endif
