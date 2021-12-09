#include "Control.hpp"
#include "Heuristica.hpp"
#include "Problem.hpp"

using namespace std;

#ifndef _GENETICO_
#define _GENETICO_

class Genetico : public Heuristica
{
public:

	int iterGenetico, tamPopGenetico, tamParticionamento;
	float probCrossOver, powerCrossOver, probMutacao;

#ifndef THREADS
	vector<Problem*> *bad_pop;
#endif

	Genetico();
	~Genetico();

	bool setParameter(const char* parameter, const char* value);

	vector<Problem*>* start(set<Problem*, bool(*)(Problem*, Problem*)>* sol, Heuristica_Listener* listener);

private:

	vector<Problem*>* exec(vector<Problem*>* pop, Heuristica_Listener* listener);
};

#endif
