#include "Control.hpp"
#include "Heuristic.hpp"
#include "Problem.hpp"

using namespace std;

#ifndef _ANNEALING_
#define _ANNEALING_

class Annealing: public Heuristic
{
public:

	int maxIter, choicePolicy, elitismo;
	float initTemp, fimTemp;
	bool restoreSolution;
	float alfa;

	Annealing();
	~Annealing();

	bool setParameter(const char* parameter, const char* value);

	vector<Problem*>* start(set<Problem*, bool(*)(Problem*, Problem*)>* sol, HeuristicListener* listener);

private:

	vector<Problem*>* exec(Problem*, HeuristicListener* listener);
};

bool accept(double rand, double Ds, double T);

#endif
