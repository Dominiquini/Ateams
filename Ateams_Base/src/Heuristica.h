#include "Problema.h"

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

	Heuristica(string nome) {name = nome;}
	virtual ~Heuristica() {cout << name << ": " << numExec << endl;}

	virtual vector<Problema*>* start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic, Heuristica_Listener* listener) = 0;
};

class Heuristica_Listener
{
public:

	double bestInitialFitness, bestActualFitness;
	Heuristica* heuristica;
	string execInfo;
	double status;
	string info;
	int id;

	Heuristica_Listener(Heuristica* alg, int threadId)
	{
		this->heuristica = alg;

		char infoThread[16];
		sprintf(infoThread, "%s(%.3d)", alg->name.c_str(), threadId);

		this->info = infoThread;

		this->id = threadId;

		this->status = -1;
	}

	inline void setInfo(char* info)
	{
		pthread_mutex_lock(&mutex_exec);

		this->execInfo = string(info);

		pthread_mutex_unlock(&mutex_exec);
	}

	inline string getInfo()
	{
		pthread_mutex_lock(&mutex_exec);

		string info = this->execInfo;

		pthread_mutex_unlock(&mutex_exec);

		return info;
	}
};

#endif
