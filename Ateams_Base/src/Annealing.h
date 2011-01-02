#include "Problema.h"
#include "Heuristica.h"
#include "Controle.h"

using namespace std;

#ifndef _ANNEALING_
#define _ANNEALING_

class Annealing: public Heuristica
{
public:

	int maxIter, polEscolha, elitismo;
	float initTemp, fimTemp;
	bool restauraSol;
	float alfa;

	Annealing();
	Annealing(string, ParametrosHeuristicas&);
	virtual ~Annealing();

	vector<Problema*>* start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic);

private:

	vector<Problema*>* exec(Problema*);
};

bool accept(double rand, double Ds, double T);

#endif
