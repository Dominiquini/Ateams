#include "Problema.hpp"

using namespace std;

extern pthread_mutex_t mutex_pop;
extern pthread_mutex_t mutex_exec;

extern volatile bool PARAR;

#ifndef _HEURISTICA_
#define _HEURISTICA_

class Heuristica_Listener;

class Heuristica
{
public:

	static int numHeuristic;

	int numExec;
	string name;
	int prob, polEscolha;

	Heuristica(string heuristicName) {name = heuristicName;}
	virtual ~Heuristica() {cout << name << ": " << numExec << endl;}

	virtual bool setParameter(const char* parameter, const char* value)
	{
		if(strcasecmp(parameter, "name") != 0)
			return false;

		name = string(value);

		return true;
	}

	virtual vector<Problema*>* start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, Heuristica_Listener* listener) = 0;
};

class Heuristica_Listener
{
public:

	double bestInitialFitness, bestActualFitness;
	Heuristica* heuristica;
	char* execInfo;
	double status;
	string info;
	int id;

	Heuristica_Listener(Heuristica* alg, int threadId)
	{
		this->heuristica = alg;

		char infoThread[16];
		sprintf(infoThread, "%s(%.3d)", alg->name.c_str(), threadId);

		this->execInfo = new char[512];
		this->info = infoThread;

		this->id = threadId;

		this->status = -1;
	}

	~Heuristica_Listener()
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
