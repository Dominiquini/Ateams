#include "Problema.h"

using namespace std;

#ifndef _HEURISTICA_
#define _HEURISTICA_

extern pthread_mutex_t mutex;

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
