#include "Problem.hpp"

using namespace std;

#ifndef _HEURISTIC_
#define _HEURISTIC_

extern mutex mutex_population;

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

	static list<HeuristicExecutionInfo*> *allHeuristics;			// Algoritmos executados ate o momento
	static list<HeuristicExecutionInfo*> *runningHeuristics;		// Algoritmos em execucao no momento

	static inline void allocateMemory();

	static inline void deallocateMemory();

	static inline void heuristicStarted(HeuristicExecutionInfo *info);

	static inline void heuristicFinished(HeuristicExecutionInfo *info);

	static inline Heuristic* selectOpportunisticHeuristic(vector<Heuristic*> *heuristics, unsigned int probTotal);

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

public:

	unsigned int totalExecutionCounter;

	milliseconds totalExecutionTime;

	Heuristic() {
		totalExecutionCounter = 0;
		totalExecutionTime = milliseconds::zero();
	}

	virtual ~Heuristic() {
		totalExecutionCounter = 0;
		totalExecutionTime = milliseconds::zero();
	}

	virtual HeuristicParameters getParameters() = 0;

	virtual bool setParameter(const char *parameter, const char *value) = 0;

	virtual void printStatistics(int total) {
		const char *heuristicName = getParameters().name.c_str();
		int heuristicChoiceProbability = getParameters().choiceProbability;
		float heuristicTotalExecutionPercentage = total != 0 ? (100.0 * totalExecutionCounter) / total : 0.0;
		int heuristicTotalTime = duration_cast<seconds>(totalExecutionTime).count();
		int heuristicAverageTime = total != 0 ? duration_cast<milliseconds>(totalExecutionTime).count() / total : 0;

		if (total == 0) {
			printf(" - %s [%03d] :: %03d (%% ------ %%) :: %04ds (~ ------ ~)\n", heuristicName, heuristicChoiceProbability, totalExecutionCounter, heuristicTotalTime);
		} else {
			printf(" - %s [%03d] :: %03d (%% %06.2f %%) :: %04ds (~ %04dms ~)\n", heuristicName, heuristicChoiceProbability, totalExecutionCounter, heuristicTotalExecutionPercentage, heuristicTotalTime, heuristicAverageTime);
		}
	}

	virtual set<Problem*>::const_iterator selectOpportunisticSolution(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) = 0;

	virtual vector<Problem*>* start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicExecutionInfo *listener) = 0;

	virtual milliseconds updateExecutionTime(steady_clock::time_point startTime, steady_clock::time_point endTime) = 0;

	virtual void markSolutions(vector<Problem*> *solutions) = 0;

private:

	static string getHeuristicNames(list<HeuristicExecutionInfo*> *heuristicsList);
};

class HeuristicExecutionInfo {
public:

	Heuristic *heuristic;

	unsigned int executionId;
	thread::id threadId;

	milliseconds executionTime;

	int contribution;

	char heuristicInfo[HEURISTIC_NAME_MAX_LENGTH];

	double bestInitialFitness, bestActualFitness;

	double status;

	char execInfo[HEURISTIC_INFO_MAX_LENGTH];

	static unsigned int getContributionSum(list<HeuristicExecutionInfo*> infos) {
		int contributionSum = 0;

		for (list<HeuristicExecutionInfo*>::iterator it = infos.begin(); it != infos.end(); it++) {
			HeuristicExecutionInfo *info = *it;

			if (info != NULL) {
				contributionSum += info->contribution;
			}
		}

		return contributionSum;
	}

	static milliseconds getTotalExecutionTime(list<HeuristicExecutionInfo*> infos) {
		milliseconds totalExecutionTime = milliseconds::zero();

		for (list<HeuristicExecutionInfo*>::iterator it = infos.begin(); it != infos.end(); it++) {
			HeuristicExecutionInfo *info = *it;

			if (info != NULL) {
				totalExecutionTime += info->executionTime;
			}
		}

		return totalExecutionTime;
	}

	HeuristicExecutionInfo(Heuristic *heuristic, unsigned int executionId, thread::id threadId) {
		this->heuristic = heuristic;

		this->executionId = executionId;
		this->threadId = threadId;

		this->bestInitialFitness = 0;
		this->bestActualFitness = 0;

		this->status = -1.0;

		this->contribution = 0;

		configHeuristicName();

		setupInfo("");
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

	inline bool isRunning() {
		return isStarted() && !isFinished();
	}

private:

	inline void configHeuristicName() {
		if (heuristic != NULL) {
			snprintf(heuristicInfo, HEURISTIC_NAME_MAX_LENGTH, "%s(%04d)", heuristic->getParameters().name.c_str(), executionId);
		}
	}
};

inline void Heuristic::allocateMemory() {
	allHeuristics = new list<HeuristicExecutionInfo*>;
	runningHeuristics = new list<HeuristicExecutionInfo*>;
}

inline void Heuristic::deallocateMemory() {
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

inline void Heuristic::heuristicStarted(HeuristicExecutionInfo *info) {
	allHeuristics->push_back(info);
	runningHeuristics->push_back(info);
}

inline void Heuristic::heuristicFinished(HeuristicExecutionInfo *info) {
	list<HeuristicExecutionInfo*>::iterator executed = find(runningHeuristics->begin(), runningHeuristics->end(), info);
	runningHeuristics->erase(executed);
}

/* Seleciona uma heuristica da lista aleatoriamente, mas diretamente proporcional a sua prioridade */
inline Heuristic* Heuristic::selectOpportunisticHeuristic(vector<Heuristic*> *heuristics, unsigned int probTotal) {
	if (heuristics == NULL || heuristics->size() == 0) {
		throw string("No Heuristics Defined!");
	}

	unsigned int sum = probTotal;
	unsigned int randWheel = Random::randomNumber(0, sum);

	for (int i = 0; i < (int) heuristics->size(); i++) {
		sum -= heuristics->at(i)->getParameters().choiceProbability;
		if (sum <= randWheel) {
			return heuristics->at(i);
		}
	}

	throw string("No Heuristic Selected!");

	return heuristics->at(0);
}

#endif
