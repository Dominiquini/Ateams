#include "Problem.hpp"

using namespace std;

#ifndef _HEURISTIC_
#define _HEURISTIC_

#define MAX_ATTEMPTS 50

extern pthread_mutex_t mutex_pop;
extern pthread_mutex_t mutex_exec;

extern volatile TerminationInfo STATUS;

class HeuristicListener;

class Heuristic {
public:

	static int heuristicsAvailable;

	static bool comparator(Heuristic *h1, Heuristic *h2) {
		return h1->choiceProbability < h2->choiceProbability;
	}

	string name;
	int executionCounter = 0;
	int choiceProbability = 0, choicePolicy = 0;

	Heuristic(string heuristicName) {
		name = heuristicName;
	}

	virtual ~Heuristic() {
	}

	virtual void printStatistics(char bullet) {
		printf(" %c %s : %03d || %% %02d %%\n", bullet, name.c_str(), executionCounter, choiceProbability);
//		cout << " " << bullet << " " << name << ": " << executionCounter << " || " << "%" << choiceProbability << "%" << endl;
	}

	virtual bool setParameter(const char *parameter, const char *value) {
		if (strcasecmp(parameter, "name") != 0)
			return false;

		name = string(value);

		return true;
	}

	virtual set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) = 0;

	virtual vector<Problem*>* start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicListener *listener) = 0;

	virtual void markSolutions(vector<Problem*>* solutions) = 0;
};

class HeuristicListener {
public:

	double bestInitialFitness, bestActualFitness;
	Heuristic *heuristic;
	char threadInfo[16];
	char execInfo[512];
	double status;
	int id;

	HeuristicListener(Heuristic *alg, int threadId) {
		this->bestInitialFitness = 0;
		this->bestActualFitness = 0;
		this->status = -1;

		this->id = threadId;

		this->heuristic = alg;

		sprintf(threadInfo, "%s(%.3d)", alg->name.c_str(), threadId);
	}

	~HeuristicListener() {
	}

	inline void setInfo(const char *format, ...) {
		pthread_mutex_lock(&mutex_exec);

		va_list args;
		va_start(args, format);
		vsprintf(this->execInfo, format, args);
		va_end(args);

		pthread_mutex_unlock(&mutex_exec);
	}

	inline const char* getInfo() {
		pthread_mutex_lock(&mutex_exec);

		char *info = this->execInfo;

		pthread_mutex_unlock(&mutex_exec);

		return info;
	}
};

#endif
