#include "Problema.h"

using namespace std;

extern pthread_mutex_t mutex;

extern volatile bool PARAR;

#ifndef _HEURISTICA_
#define _HEURISTICA_

class Heuristica
{
public:

	static int numHeuristic;

	int numExec;
	string name;
	int prob, polEscolha;

	Heuristica(string nome) {name = nome;}
	virtual ~Heuristica() {cout << name << ": " << numExec << endl;}

	virtual vector<Problema*>* start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic) = 0;
};

#endif
