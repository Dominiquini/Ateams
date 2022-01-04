#include "HeuristicTabu.hpp"

using namespace std;

TabuSearch::TabuSearch() : Heuristic::Heuristic("DEFAULT_BT") {
	executionCounter = 0;

	choiceProbability = 35;
	funcAsp = 0.5;
	choicePolicy = 100;
	iterTabu = 750;
	tamListaTabu = 10;
	tentSemMelhora = 500;
	polExploracao = 0.5;
	elitism = 10;

	Heuristic::heuristicsProbabilitySum += choiceProbability;
}

TabuSearch::~TabuSearch() {
	Heuristic::heuristicsProbabilitySum -= choiceProbability;
}

bool TabuSearch::setParameter(const char *parameter, const char *value) {
	if (Heuristic::setParameter(parameter, value))
		return true;

	if (strcasecmp(parameter, "probBT") == 0) {
		Heuristic::heuristicsProbabilitySum -= choiceProbability;
		sscanf(value, "%d", &choiceProbability);
		Heuristic::heuristicsProbabilitySum += choiceProbability;
	} else if (strcasecmp(parameter, "choicePolicyTS") == 0) {
		sscanf(value, "%d", &choicePolicy);
	} else if (strcasecmp(parameter, "elitismProbabilityTS") == 0) {
		sscanf(value, "%d", &elitism);
	} else if (strcasecmp(parameter, "aspirationCriteriaTS") == 0) {
		sscanf(value, "%f", &funcAsp);
	} else if (strcasecmp(parameter, "iterationsTS") == 0) {
		sscanf(value, "%d", &iterTabu);
	} else if (strcasecmp(parameter, "attemptsWithoutImprovementTS") == 0) {
		sscanf(value, "%d", &tentSemMelhora);
	} else if (strcasecmp(parameter, "listSizeTS") == 0) {
		sscanf(value, "%d", &tamListaTabu);
	} else if (strcasecmp(parameter, "explorationPolicyTS") == 0) {
		sscanf(value, "%f", &polExploracao);
	} else {
		return false;
	}

	return true;
}

/* Executa uma Busca Tabu na populacao com o devido criterio de selecao */
vector<Problem*>* TabuSearch::start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicExecutionInfo *listener) {
	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator selection;
	Problem *solBT;

	pthread_mutex_lock(&mutex_pop);

	executionCounter++;

	// Escolhe a melhor solucao para ser usada pelo BT
	if (choicePolicy == 0 || random(0, 101) < elitism) {
		selection = sol->begin();
	} else {
		double fitTotal = choicePolicy < 0 ? Control::sumFitnessMaximize(sol, sol->size()) : Control::sumFitnessMaximize(sol, choicePolicy);

		selection = selectRouletteWheel(sol, fitTotal);
	}

	solBT = Problem::copySolution(**selection);

	pthread_mutex_unlock(&mutex_pop);

	return exec(solBT, listener);
}

set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator TabuSearch::selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) {
	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator selection = population->begin();
	int attemps = 0;

	while ((selection == population->begin() || (*selection)->heuristicsInfo.tabu == true) && (attemps++ < MAX_ATTEMPTS))
		selection = Control::selectRouletteWheel(population, fitTotal);

	(*selection)->heuristicsInfo.tabu++;

	return selection;
}

void TabuSearch::markSolutions(vector<Problem*> *solutions) {
	for (auto solution = solutions->cbegin(); solution != solutions->cend(); ++solution) {
		(*solution)->heuristicsInfo.tabu++;
	}
}

vector<Problem*>* TabuSearch::exec(Problem *init, HeuristicExecutionInfo *listener) {
	vector<pair<Problem*, InfoTabu*>*> *vizinhanca;
	pair<Problem*, InfoTabu*> *local;

	// Lista Tabu de movimentos repetidos
	list<InfoTabu*> *listaTabu = new list<InfoTabu*>;

	// Maximos globais e locais na execucao da Busca Tabu
	vector<Problem*> *maxGlobal = new vector<Problem*>();
	Problem *maxLocal = init;

	maxGlobal->push_back(Problem::copySolution(*maxLocal));

	if (listener != NULL)
		listener->bestInitialFitness = (*maxGlobal->begin())->getFitness();

	// Loop principal
	for (int i = 0, j = 0; i < iterTabu && j < tentSemMelhora; i++, j++) {
		if (STATUS != EXECUTING)
			break;

		if (listener != NULL) {
			listener->status = (100.00 * (double) (i + 1)) / (double) iterTabu;

			listener->bestActualFitness = (*maxGlobal->rbegin())->getFitness();

			listener->setupInfo("Iteration: %d", i + 1);
		}

		if (polExploracao <= 0 || polExploracao >= 1) {
			// Pega uma lista de todos os "vizinhos" de maxLocal
			vizinhanca = maxLocal->localSearch();
		} else {
			// Pega uma parcela 'polExploracao' dos "vizinhos" de maxLocal
			vizinhanca = maxLocal->localSearch(polExploracao);
		}

		// Escolhe a solucao de peso minimo
		while (!vizinhanca->empty()) {
			local = vizinhanca->back();
			vizinhanca->pop_back();

			// Se nao for tabu...
			if (!isTabu(listaTabu, local->second)) {
				if (Problem::improvement(*maxLocal, *local->first) > 0)
					j = 0;

				delete maxLocal;
				maxLocal = local->first;

				if (Problem::improvement(*maxGlobal->back(), *local->first) > 0) {
					maxGlobal->push_back(Problem::copySolution(*maxLocal));
				}

				addTabu(listaTabu, local->second, tamListaTabu);

				delete local;

				break;
			}
			// Eh tabu...
			else {
				// Satisfaz a funcao de aspiracao
				if (aspiracao((double) funcAsp, local->first, maxLocal, maxGlobal->back())) {
					j = 0;

					delete maxLocal;
					maxLocal = local->first;

					if (Problem::improvement(*maxGlobal->back(), *local->first) > 0) {
						maxGlobal->push_back(Problem::copySolution(*maxLocal));
					}

					delete local->second;
					delete local;

					break;
				} else {
					delete local->first;
					delete local->second;
					delete local;
				}
			}
		}
		while (!vizinhanca->empty()) {
			local = vizinhanca->back();
			vizinhanca->pop_back();

			delete local->first;
			delete local->second;
			delete local;
		}
		vizinhanca->clear();
		delete vizinhanca;
	}
	delete maxLocal;

	while (!listaTabu->empty()) {
		delete listaTabu->back();
		listaTabu->pop_back();
	}
	listaTabu->clear();
	delete listaTabu;

	markSolutions(maxGlobal);

	return maxGlobal;
}

/* Verdadeiro se movimento avaliado for Tabu */
inline bool isTabu(list<InfoTabu*> *listaTabu, InfoTabu *m) {
	list<InfoTabu*>::iterator iter;

	for (iter = listaTabu->begin(); iter != listaTabu->end(); iter++)
		if (*m == **iter)
			return true;

	return false;
}

inline void addTabu(list<InfoTabu*> *listaTabu, InfoTabu *m, int max) {
	listaTabu->push_front(m);

	if ((int) listaTabu->size() > max) {
		delete listaTabu->back();
		listaTabu->pop_back();
	}
}

inline bool aspiracao(double paramAsp, Problem *atual, Problem *local, Problem *global) {
	return Problem::improvement(((paramAsp * global->getFitness()) + ((1 - paramAsp) * local->getFitness())), atual->getFitness()) >= 0;
}
