#include "Problem.hpp"

using namespace std;

#ifndef _HEURISTIC_
#define _HEURISTIC_

extern pthread_mutex_t mutex_pop;
extern pthread_mutex_t mutex_exec;

extern volatile bool TERMINATE;

class HeuristicListener;

class Heuristic
{
public:

	static int heuristicsAvailable;

	static bool comparator(Heuristic *h1, Heuristic *h2){
		return h1->choiceProbability < h2->choiceProbability;
	}

	string name;
	int executionCounter = 0;
	int choiceProbability = 0, choicePolicy = 0;

	Heuristic(string heuristicName) {name = heuristicName;}
	virtual ~Heuristic() {cout << name << ": " << executionCounter << endl;}

	virtual bool setParameter(const char* parameter, const char* value)
	{
		if(strcasecmp(parameter, "name") != 0)
			return false;

		name = string(value);

		return true;
	}

	virtual vector<Problem*>* start(set<Problem*, bool(*)(Problem*, Problem*)>* sol, HeuristicListener* listener) = 0;
};

class HeuristicListener
{
public:

	double bestInitialFitness, bestActualFitness;
	Heuristic* heuristica;
	char* execInfo;
	double status;
	string info;
	int id;

	HeuristicListener(Heuristic* alg, int threadId)
	{
		this->bestInitialFitness = 0;
		this->bestActualFitness = 0;

		this->heuristica = alg;

		char infoThread[16];
		sprintf(infoThread, "%s(%.3d)", alg->name.c_str(), threadId);

		this->execInfo = new char[512];
		this->info = infoThread;

		this->id = threadId;

		this->status = -1;
	}

	~HeuristicListener()
	{
		if(execInfo != NULL)
			delete[] execInfo;
	}

	inline void setInfo(const char* format, ...)
	{
		pthread_mutex_lock(&mutex_exec);

		va_list args;
		va_start(args, format);
		vsprintf(this->execInfo, format, args);
		va_end(args);

		pthread_mutex_unlock(&mutex_exec);
	}

	inline const char* getInfo()
	{
		pthread_mutex_lock(&mutex_exec);

		char* info = this->execInfo;

		pthread_mutex_unlock(&mutex_exec);

		return info;
	}
};

#endif
