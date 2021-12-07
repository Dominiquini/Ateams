#include "Problema.hpp"
#include "Heuristica.hpp"
#include "Controle.hpp"

using namespace std;

#ifndef _GENETICO_
#define _GENETICO_

class Genetico : public Heuristica
{
public:

	int iterGenetico, tamPopGenetico, tamParticionamento;
	float probCrossOver, powerCrossOver, probMutacao;

#ifndef THREADS
	vector<Problema*> *bad_pop;
#endif

	Genetico();
	~Genetico();

	bool setParameter(const char* parameter, const char* value);

	vector<Problema*>* start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, Heuristica_Listener* listener);

private:

	vector<Problema*>* exec(vector<Problema*>* pop, Heuristica_Listener* listener);
};

#endif
