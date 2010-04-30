#include "Problema.h"
#include "Heuristica.h"

using namespace std;

#ifndef _GENETICO_
#define _GENETICO_

class Genetico : public Heuristica
{
public:

	int iterGenetico, tamPopGenetico, probCrossOver, probMutacao;

	Genetico();
	Genetico(ParametrosAG*);

	vector<Problema*>* start(set<Problema*, bool(*)(Problema*, Problema*)>* sol);

private:

	vector<Problema*>* exec(Problema*);
};

#endif
