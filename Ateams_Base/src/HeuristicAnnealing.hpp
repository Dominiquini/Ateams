#include "Control.hpp"
#include "Heuristic.hpp"
#include "Problem.hpp"

using namespace std;

#ifndef _ANNEALING_
#define _ANNEALING_

#define SIMULATED_ANNEALING_NAME "SimulatedAnnealing"

struct SimulatedAnnealingParameters : HeuristicParameters {

	map<string, void*> simulatedAnnealingKeys = {
			{"probSA", &choiceProbability},
			{"choicePolicySA", &choicePolicy},
			{"maxIterationsSA", &maxIterations},
			{"elitismProbabilitySA", &elitismProbability},
			{"startTempSA", &startTemp},
			{"endTempSA", &endTemp},
			{"alphaSA", &alpha}
	};

	int maxIterations = 250;

	float elitismProbability = 0.25;

	float startTemp = 125.0;
	float endTemp = 0.75;
	float alpha = 0.99;

	SimulatedAnnealingParameters() {
		keys.insert(simulatedAnnealingKeys.begin(), simulatedAnnealingKeys.end());
	}
};

class SimulatedAnnealing: public Heuristic {
public:

	SimulatedAnnealingParameters parameters;

	SimulatedAnnealing();
	~SimulatedAnnealing();

	HeuristicParameters getParameters() override;

	bool setParameter(const char *parameter, const char *value) override;

	set<Problem*>::const_iterator selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) override;

	vector<Problem*>* start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicExecutionInfo *listener) override;

	void markSolutions(vector<Problem*>* solutions) override;

private:

	vector<Problem*>* exec(Problem*, HeuristicExecutionInfo *listener);
};

bool accept(double rand, double Ds, double T);

#endif
