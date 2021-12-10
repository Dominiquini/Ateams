#include "Annealing.hpp"

using namespace std;

Annealing::Annealing() : Heuristic::Heuristic("DEFAULT_SA") {
	executionCounter = 0;

	choiceProbability = 52;
	choicePolicy = 10;
	maxIter = 250;
	initTemp = 125;
	fimTemp = 0.75;
	restoreSolution = false;
	alfa = 0.99;
	elitismo = 20;

	Heuristic::heuristicsAvailable += choiceProbability;
}

Annealing::~Annealing() {
	Heuristic::heuristicsAvailable -= choiceProbability;
}

bool Annealing::setParameter(const char *parameter, const char *value) {
	if (Heuristic::setParameter(parameter, value))
		return true;

	if (strcasecmp(parameter, "probSA") == 0) {
		Heuristic::heuristicsAvailable -= choiceProbability;
		sscanf(value, "%d", &choiceProbability);
		Heuristic::heuristicsAvailable += choiceProbability;
	} else if (strcasecmp(parameter, "choicePolicySA") == 0) {
		sscanf(value, "%d", &choicePolicy);
	} else if (strcasecmp(parameter, "elitismProbabilitySA") == 0) {
		sscanf(value, "%d", &elitismo);
	} else if (strcasecmp(parameter, "maxIterationsSA") == 0) {
		sscanf(value, "%d", &maxIter);
	} else if (strcasecmp(parameter, "startTempSA") == 0) {
		sscanf(value, "%f", &initTemp);
	} else if (strcasecmp(parameter, "endTempSA") == 0) {
		sscanf(value, "%f", &fimTemp);
	} else if (strcasecmp(parameter, "restoreSolutionSA") == 0) {
		int temp;

		sscanf(value, "%d", &temp);

		if (temp == 1)
			restoreSolution = true;
		else
			restoreSolution = false;
	} else if (strcasecmp(parameter, "alphaSA") == 0) {
		sscanf(value, "%f", &alfa);
	} else {
		return false;
	}

	return true;
}

/* Executa um Simulated Annealing na populacao com o devido criterio de selecao */
vector<Problem*>* Annealing::start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicListener *listener) {
	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator select;
	Problem *solSA;

	executionCounter++;

	pthread_mutex_lock(&mutex_pop);

	// Escolhe a melhor solucao para ser usada pelo SA
	if (choicePolicy == 0 || xRand(0, 101) < elitismo) {
		select = sol->begin();
		solSA = Problem::copySolution(**select);

		pthread_mutex_unlock(&mutex_pop);

		return exec(solSA, listener);
	}

	// Escolhe alguem dentre os 'choicePolicy' primeiras solucoes
	double visao = choicePolicy < 0 ? Control::sumFitnessMaximize(sol, sol->size()) : Control::sumFitnessMaximize(sol, choicePolicy);

	// Evita trabalhar sobre solucoes ja selecionadas anteriormente
	select = Control::selectRouletteWheel(sol, visao);
	if (choicePolicy < -1) {
		while ((*select)->exec.annealing == true) {
			if (select != sol->begin())
				select--;
			else
				break;
		}
	}

	(*select)->exec.annealing = true;

	solSA = Problem::copySolution(**select);

	pthread_mutex_unlock(&mutex_pop);

	return exec(solSA, listener);
}

/* Executa uma busca por solucoes a partir de 'init' */
vector<Problem*>* Annealing::exec(Problem *Si, HeuristicListener *listener) {
	vector<Problem*> *Sf = new vector<Problem*>();
	Problem *S, *Sn;
	double Ti, Tf, T;
	int L = maxIter;
	double Ds;

	Ti = initTemp != -1 ? initTemp : (Problem::best - Problem::worst) / log(0.5);
	Tf = fimTemp > 0 ? fimTemp : 1;

	double diff = Ti - Tf;

	T = Ti;
	S = Si;

	Sf->push_back(Problem::copySolution(*Si));

	if (listener != NULL)
		listener->bestInitialFitness = (*Sf->begin())->getFitness();

	bool exec = true;
	while (exec) {
		if (TERMINATE == true)
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

		if (restoreSolution) {
			delete S;
			S = Problem::copySolution(*Sf->back());
		}
	}
	delete S;

	return Sf;
}

inline bool accept(double rand, double Ds, double T) {
	return ((rand / ((float) RAND_MAX)) < (exp(Ds / T)));
}
