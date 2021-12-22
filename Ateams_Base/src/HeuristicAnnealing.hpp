#include "Control.hpp"
#include "Heuristic.hpp"
#include "Problem.hpp"

using namespace std;

#ifndef _ANNEALING_
#define _ANNEALING_

class SimulatedAnnealing: public Heuristic
{
public:

	int maxIter, elitism;
	float startTemp, endTemp;
	bool restoreSolution;
	float alfa;

	SimulatedAnnealing();
	~SimulatedAnnealing();

	bool setParameter(const char* parameter, const char* value);

	vector<Problem*>* start(set<Problem*, bool(*)(Problem*, Problem*)>* sol, HeuristicListener* listener);

private:

	vector<Problem*>* exec(Problem*, HeuristicListener* listener);
};

bool accept(double rand, double Ds, double T);

#endif
