#include "HeuristicAnnealing.hpp"

using namespace std;

SimulatedAnnealing::SimulatedAnnealing() : Heuristic::Heuristic("DEFAULT_SA") {
	executionCounter = 0;

	choiceProbability = 52;
	choicePolicy = 10;
	maxIter = 250;
	startTemp = 125;
	endTemp = 0.75;
	alfa = 0.99;
	elitism = 20;

	Heuristic::heuristicsAvailable += choiceProbability;
}

SimulatedAnnealing::~SimulatedAnnealing() {
	Heuristic::heuristicsAvailable -= choiceProbability;
}

bool SimulatedAnnealing::setParameter(const char *parameter, const char *value) {
	if (Heuristic::setParameter(parameter, value))
		return true;

	if (strcasecmp(parameter, "probSA") == 0) {
		Heuristic::heuristicsAvailable -= choiceProbability;
		sscanf(value, "%d", &choiceProbability);
		Heuristic::heuristicsAvailable += choiceProbability;
	} else if (strcasecmp(parameter, "choicePolicySA") == 0) {
		sscanf(value, "%d", &choicePolicy);
	} else if (strcasecmp(parameter, "elitismProbabilitySA") == 0) {
		sscanf(value, "%d", &elitism);
	} else if (strcasecmp(parameter, "maxIterationsSA") == 0) {
		sscanf(value, "%d", &maxIter);
	} else if (strcasecmp(parameter, "startTempSA") == 0) {
		sscanf(value, "%f", &startTemp);
	} else if (strcasecmp(parameter, "endTempSA") == 0) {
		sscanf(value, "%f", &endTemp);
	} else if (strcasecmp(parameter, "alphaSA") == 0) {
		sscanf(value, "%f", &alfa);
	} else {
		return false;
	}

	return true;
}

/* Executa um Simulated Annealing na populacao com o devido criterio de selecao */
vector<Problem*>* SimulatedAnnealing::start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicListener *listener) {
	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator selection;
	Problem *solSA;

	pthread_mutex_lock(&mutex_pop);

	executionCounter++;

	// Escolhe a melhor solucao para ser usada pelo SA
	if (choicePolicy == 0 || xRand(0, 101) < elitism) {
		selection = sol->begin();
	} else {
		double fitTotal = choicePolicy < 0 ? Control::sumFitnessMaximize(sol, sol->size()) : Control::sumFitnessMaximize(sol, choicePolicy);

		selection = selectRouletteWheel(sol, fitTotal);
	}

	solSA = Problem::copySolution(**selection);

	pthread_mutex_unlock(&mutex_pop);

	return exec(solSA, listener);
}

set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator SimulatedAnnealing::selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) {
	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator selection = population->begin();
	int attemps = 0;

	while ((selection == population->begin() || (*selection)->heuristicsInfo.annealing == true) && (attemps++ < MAX_ATTEMPTS))
		selection = Control::selectRouletteWheel(population, fitTotal);

	(*selection)->heuristicsInfo.annealing++;

	return selection;
}

void SimulatedAnnealing::markSolutions(vector<Problem*> *solutions) {
	for (auto solution = solutions->cbegin(); solution != solutions->cend(); ++solution) {
		(*solution)->heuristicsInfo.annealing++;
	}
}

vector<Problem*>* SimulatedAnnealing::exec(Problem *Si, HeuristicListener *listener) {
	vector<Problem*> *Sf = new vector<Problem*>();
	Problem *S, *Sn;
	double Ti, Tf, T;
	int L = maxIter;
	double Ds;

	Ti = startTemp != -1 ? startTemp : (Problem::best - Problem::worst) / log(0.5);
	Tf = endTemp > 0 ? endTemp : 1;

	double diff = Ti - Tf;

	T = Ti;
	S = Si;

	Sf->push_back(Problem::copySolution(*Si));

	if (listener != NULL)
		listener->bestInitialFitness = (*Sf->begin())->getFitness();

	bool exec = true;

	while (exec) {
		if (STATUS != EXECUTING)
			break;

		if (listener != NULL) {
			listener->status = 100.00 - (100.00 * (T - Tf)) / diff;

			listener->bestActualFitness = (*Sf->rbegin())->getFitness();

			listener->setInfo("Temperature: %f", T);
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
			if (Ds >= 0 || accept((double) xRand(), Ds, T)) {
				delete S;
				S = Sn;

				if (Problem::improvement(*Sf->back(), *S) > 0) {
					Sf->push_back(Problem::copySolution(*S));
				}
			} else {
				delete Sn;
			}
		}

		T = alfa * T;

		if (T < Tf)
			T = Tf;
	}

	delete S;

	markSolutions(Sf);

	return Sf;
}

inline bool accept(double rand, double Ds, double T) {
	return ((rand / ((float) RAND_MAX)) < (exp(Ds / T)));
}
