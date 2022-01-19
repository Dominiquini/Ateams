#include "HeuristicAnnealing.hpp"

using namespace std;

SimulatedAnnealing::SimulatedAnnealing() : Heuristic::Heuristic() {
}

SimulatedAnnealing::~SimulatedAnnealing() {
}

HeuristicParameters SimulatedAnnealing::getParameters() {
	return parameters;
}

bool SimulatedAnnealing::setParameter(const char *parameter, const char *value) {
	return parameters.setParameter(parameter, value);
}

/* Executa um Simulated Annealing na populacao com o devido criterio de selecao */
vector<Problem*>* SimulatedAnnealing::start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicExecutionInfo *info) {
	set<Problem*>::const_iterator selection;
	Problem *solSA;

	{
		scoped_lock<decltype(mutex_population)> lock_population(mutex_population);

		totalExecutionCounter++;

		// Escolhe a melhor solucao para ser usada pelo SA
		if (parameters.choicePolicy == 0 || Random::randomPercentage() < (100 * parameters.elitismProbability)) {
			selection = sol->cbegin();
		} else {
			double fitTotal = parameters.choicePolicy < 0 ? Problem::sumFitnessMaximize(sol, sol->size()) : Problem::sumFitnessMaximize(sol, parameters.choicePolicy);

			selection = selectOpportunisticSolution(sol, fitTotal);
		}

		solSA = Problem::copySolution(**selection);
	}

	return exec(solSA, info);
}

set<Problem*>::const_iterator SimulatedAnnealing::selectOpportunisticSolution(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) {
	set<Problem*>::const_iterator selection = population->cbegin();
	int attemps = 0;

	while ((selection == population->cbegin() || (*selection)->heuristicsCounter.annealing == true) && (attemps++ < HEURISTIC_SELECTION_MAX_ATTEMPTS))
		selection = Problem::selectOpportunisticSolution(population, fitTotal);

	(*selection)->heuristicsCounter.annealing++;

	return selection;
}

milliseconds SimulatedAnnealing::updateExecutionTime(steady_clock::time_point startTime, steady_clock::time_point endTime) {
	milliseconds executionTime = duration_cast<milliseconds>(endTime - startTime);

	totalExecutionTime += executionTime;

	return executionTime;
}

void SimulatedAnnealing::markSolutions(vector<Problem*> *solutions) {
	for (auto solution = solutions->cbegin(); solution != solutions->cend(); ++solution) {
		(*solution)->heuristicsCounter.annealing++;
	}
}

vector<Problem*>* SimulatedAnnealing::exec(Problem *Si, HeuristicExecutionInfo *info) {
	vector<Problem*> *Sf = new vector<Problem*>();
	Problem *S, *Sn;
	double Ti, Tf, T;
	int L = parameters.maxIterations;
	double Ds;

	steady_clock::time_point startTime = steady_clock::now();

	Ti = parameters.startTemp != -1 ? parameters.startTemp : (Problem::best - Problem::worst) / log(0.5);
	Tf = parameters.endTemp > 0 ? parameters.endTemp : 1;

	double diff = Ti - Tf;

	T = Ti;
	S = Si;

	Sf->push_back(Problem::copySolution(*Si));

	{
		info->bestInitialFitness = (*Sf->begin())->getFitness();
		info->bestActualFitness = (*Sf->rbegin())->getFitness();

		info->status = 0.0f;

		info->setupInfo("Temperature: %f", T);
	}

	bool continueExecution = true;

	while (continueExecution) {
		if (HEURISTIC_ALLOW_TERMINATION && STATUS != EXECUTING) {
			break;
		}

		{
			info->status = 100.00 - (100.00 * (T - Tf)) / diff;
			info->bestActualFitness = (*Sf->rbegin())->getFitness();

			info->setupInfo("Temperature: %f", T);
		}

		continueExecution = T != Tf;

		for (int i = 1; i < L; i++) {
			Sn = S->neighbor();

			if (Sn == NULL) {
				i--;
				continue;
			}

			Ds = Problem::improvement(*S, *Sn);
			if (Ds >= 0 || accept((double) Random::randomNumber(), Ds, T)) {
				delete S;
				S = Sn;

				if (Problem::improvement(*Sf->back(), *S) > 0) {
					Sf->push_back(Problem::copySolution(*S));
				}
			} else {
				delete Sn;
			}
		}

		T = parameters.alpha * T;

		if (T < Tf)
			T = Tf;
	}

	delete S;

	markSolutions(Sf);

	steady_clock::time_point endTime = steady_clock::now();

	milliseconds executionTime = updateExecutionTime(startTime, endTime);

	{
		info->executionTime = executionTime;
		info->contribution = Sf->size();
	}

	return Sf;
}

inline bool accept(double rand, double Ds, double T) {
	return ((rand / ((double) RAND_MAX)) < (exp(Ds / T)));
}
