#include "JobShop.h"

#ifndef _HEURISTICA_
#define _HEURISTICA_

class Heuristica
{
public:

	static int numHeuristic;

	int prob, polEscolha;
	string name;

	virtual vector<Problema*>* start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic) = 0;
};

#endif
