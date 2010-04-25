#include "JobShop.h"

#ifndef _HEURISTICA_
#define _HEURISTICA_

class Heuristica
{
public:

	static int numHeuristic;

	int prob;
	string name;

	virtual vector<Problema*>* start(multiset<Problema*, bool(*)(Problema*, Problema*)>* sol) = 0;
};

#endif
