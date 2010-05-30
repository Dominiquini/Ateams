#include "Problema.h"
#include "Heuristica.h"
#include "Controle.h"

using namespace std;

#ifndef _GENETICO_
#define _GENETICO_

class Genetico : public Heuristica
{
public:

	int iterGenetico, tamPopGenetico, tamBadGenetico, tamParticionamento;
	float probCrossOver, probMutacao;

#ifndef THREADS
	vector<Problema*> *bad_pop;
#endif

	Genetico();
	Genetico(ParametrosAG*);
	~Genetico();

	vector<Problema*>* start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic);

private:

	vector<Problema*>* exec(vector<Problema*>* pop);
};

#endif
