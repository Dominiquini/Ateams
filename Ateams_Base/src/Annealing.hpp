#include "Control.hpp"
#include "Heuristica.hpp"
#include "Problem.hpp"

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

	vector<Problem*>* start(set<Problem*, bool(*)(Problem*, Problem*)>* sol, Heuristica_Listener* listener);

private:

	vector<Problem*>* exec(Problem*, Heuristica_Listener* listener);
};

bool accept(double rand, double Ds, double T);

#endif
