#include "Control.hpp"
#include "Heuristic.hpp"
#include "Problem.hpp"

using namespace std;

#ifndef _GENETIC_
#define _GENETIC_

#define GENETIC_ALGORITHM_NAME "GeneticAlgorithm"

struct GeneticAlgorithmParameters : HeuristicParameters {

	map<string, void*> geneticAlgorithmKeys = {
			{"probAG", &choiceProbability},
			{"choicePolicyGA", &choicePolicy},
			{"iterationsGA", &iterations},
			{"populationSizeAG", &populationSize},
			{"partitionSizeAG", &partitionSize},
			{"crossoverProbabilityAG", &crossoverProbability},
			{"crossoverPowerAG", &crossoverPower},
			{"mutationProbabilityAG", &mutationProbability}
	};

	int iterations = 500;
	int populationSize = 250;
	int partitionSize = 0;

	float crossoverProbability = 0.8;
	float crossoverPower = 0.5;
	float mutationProbability = 0.08;

	GeneticAlgorithmParameters() {
		keys.insert(geneticAlgorithmKeys.begin(), geneticAlgorithmKeys.end());
	}
};

class GeneticAlgorithm: public Heuristic {
public:

	GeneticAlgorithmParameters parameters;

	GeneticAlgorithm();
	~GeneticAlgorithm();

	HeuristicParameters getParameters() override;

	bool setParameter(const char *parameter, const char *value) override;

	set<Problem*>::const_iterator selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) override;

	vector<Problem*>* start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicExecutionInfo *listener) override;

	void markSolutions(vector<Problem*>* solutions) override;

private:

	vector<Problem*>* exec(vector<Problem*> *pop, HeuristicExecutionInfo *listener);
};

#endif
