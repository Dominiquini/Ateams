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

	pthread_mutex_lock(&mutex_pop);

	executionCounter++;

	// Escolhe a melhor solucao para ser usada pelo SA
	if (parameters.choicePolicy == 0 || randomPercentage() < (100 * parameters.elitismProbability)) {
		selection = sol->cbegin();
	} else {
		double fitTotal = parameters.choicePolicy < 0 ? Control::sumFitnessMaximize(sol, sol->size()) : Control::sumFitnessMaximize(sol, parameters.choicePolicy);

		selection = selectRouletteWheel(sol, fitTotal);
	}

	solSA = Problem::copySolution(**selection);

	pthread_mutex_unlock(&mutex_pop);

	return exec(solSA, info);
}

set<Problem*>::const_iterator SimulatedAnnealing::selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) {
	set<Problem*>::const_iterator selection = population->cbegin();
	int attemps = 0;

	while ((selection == population->cbegin() || (*selection)->heuristicsInfo.annealing == true) && (attemps++ < MAX_ATTEMPTS))
		selection = Control::selectRouletteWheel(population, fitTotal);

	(*selection)->heuristicsInfo.annealing++;

	return selection;
}

void SimulatedAnnealing::markSolutions(vector<Problem*> *solutions) {
	for (auto solution = solutions->cbegin(); solution != solutions->cend(); ++solution) {
		(*solution)->heuristicsInfo.annealing++;
	}
}

vector<Problem*>* SimulatedAnnealing::exec(Problem *Si, HeuristicExecutionInfo *info) {
	vector<Problem*> *Sf = new vector<Problem*>();
	Problem *S, *Sn;
	double Ti, Tf, T;
	int L = parameters.maxIterations;
	double Ds;

	Ti = parameters.startTemp != -1 ? parameters.startTemp : (Problem::best - Problem::worst) / log(0.5);
	Tf = parameters.endTemp > 0 ? parameters.endTemp : 1;

	double diff = Ti - Tf;

	T = Ti;
	S = Si;

	Sf->push_back(Problem::copySolution(*Si));

	if (info != NULL) {
		info->bestInitialFitness = (*Sf->begin())->getFitness();
		info->bestActualFitness = (*Sf->rbegin())->getFitness();

		info->status = 0.0f;

		info->setupInfo("Temperature: %f", T);
	}

	bool exec = true;

	while (exec) {
		if (STATUS != EXECUTING)
			break;

		if (info != NULL) {
			info->status = 100.00 - (100.00 * (T - Tf)) / diff;

			info->bestActualFitness = (*Sf->rbegin())->getFitness();

			info->setupInfo("Temperature: %f", T);
		}

		if (T == Tf)
			exec = false;

		for (int i = 1; i < L; i++) {
			Sn = S->neighbor();

			if (Sn == NULL) {
				i--;
				continue;
			}

			Ds = Problem::improvement(*S, *Sn);
			if (Ds >= 0 || accept((double) randomNumber(), Ds, T)) {
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

	if (info != NULL) {
		info->contribution = Sf->size();
	}

	return Sf;
}

inline bool accept(double rand, double Ds, double T) {
	return ((rand / ((float) RAND_MAX)) < (exp(Ds / T)));
}
