#include "Problema.h"

using namespace std;

#ifndef _HEURISTICA_
#define _HEURISTICA_

extern pthread_mutex_t mutex;

class Heuristica
{
public:

	static int numHeuristic;

	string name;
	int prob;

	virtual ~Heuristica() {}

	virtual vector<Problema*>* start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic) = 0;
};

#endif
