#include "Problema.h"
#include "Heuristica.h"
#include "Controle.h"

using namespace std;

#ifndef _ANNEALING_
#define _ANNEALING_

class Annealing: public Heuristica
{
public:

	int maxIter, initTemp, fimTemp, polEscolha;
	bool restauraSol;
	float alfa;

	Annealing();
	Annealing(ParametrosSA*);
	~Annealing();

	vector<Problema*>* start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic);

private:

	vector<Problema*>* exec(Problema*);
};

bool accept(double rand, double Ds, double T);

#endif
