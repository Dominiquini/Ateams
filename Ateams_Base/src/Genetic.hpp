#include "Control.hpp"
#include "Heuristic.hpp"
#include "Problem.hpp"

using namespace std;

#ifndef _GENETICO_
#define _GENETICO_

class Genetic : public Heuristic
{
public:

	int iterGenetico, populationSizeGenetico, tamParticionamento;
	float crossoverProbability, crossoverPowerAG, probMutacao;

	Genetic();
	~Genetic();

	bool setParameter(const char* parameter, const char* value);

	vector<Problem*>* start(set<Problem*, bool(*)(Problem*, Problem*)>* sol, HeuristicListener* listener);

private:

	vector<Problem*>* exec(vector<Problem*>* pop, HeuristicListener* listener);
};

#endif
