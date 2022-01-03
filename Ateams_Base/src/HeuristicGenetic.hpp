#include "Control.hpp"
#include "Heuristic.hpp"
#include "Problem.hpp"

using namespace std;

#ifndef _GENETIC_
#define _GENETIC_

class GeneticAlgorithm: public Heuristic {
public:

	int iterGenetico, populationSizeGenetico, tamParticionamento;
	float crossoverProbability, crossoverPowerAG, probMutacao;

	GeneticAlgorithm();
	~GeneticAlgorithm();

	bool setParameter(const char *parameter, const char *value) override;

	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) override;

	vector<Problem*>* start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicExecutionInfo *listener) override;

	void markSolutions(vector<Problem*>* solutions) override;

private:

	vector<Problem*>* exec(vector<Problem*> *pop, HeuristicExecutionInfo *listener);
};

#endif
