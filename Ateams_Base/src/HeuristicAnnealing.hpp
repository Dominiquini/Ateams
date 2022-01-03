#include "Control.hpp"
#include "Heuristic.hpp"
#include "Problem.hpp"

using namespace std;

#ifndef _ANNEALING_
#define _ANNEALING_

class SimulatedAnnealing: public Heuristic {
public:

	int maxIter, elitism;
	float startTemp, endTemp;
	float alfa;

	SimulatedAnnealing();
	~SimulatedAnnealing();

	bool setParameter(const char *parameter, const char *value) override;

	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) override;

	vector<Problem*>* start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicExecutionInfo *listener) override;

	void markSolutions(vector<Problem*>* solutions) override;

private:

	vector<Problem*>* exec(Problem*, HeuristicExecutionInfo *listener);
};

bool accept(double rand, double Ds, double T);

#endif
