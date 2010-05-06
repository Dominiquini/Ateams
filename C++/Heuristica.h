#include "JobShop.h"

#ifndef _HEURISTICA_
#define _HEURISTICA_

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
