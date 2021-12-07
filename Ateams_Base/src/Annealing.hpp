#include "Problema.hpp"
#include "Heuristica.hpp"
#include "Controle.hpp"

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
	~Annealing();

	bool setParameter(const char* parameter, const char* value);

	vector<Problema*>* start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, Heuristica_Listener* listener);

private:

	vector<Problema*>* exec(Problema*, Heuristica_Listener* listener);
};

bool accept(double rand, double Ds, double T);

#endif
