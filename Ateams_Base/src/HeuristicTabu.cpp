#include "HeuristicTabu.hpp"

using namespace std;

TabuSearch::TabuSearch() : Heuristic::Heuristic() {
}

TabuSearch::~TabuSearch() {
}

HeuristicParameters TabuSearch::getParameters() {
	return parameters;
}

bool TabuSearch::setParameter(const char *parameter, const char *value) {
	return parameters.setParameter(parameter, value);
}

/* Executa uma Busca Tabu na populacao com o devido criterio de selecao */
vector<Problem*>* TabuSearch::start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicExecutionInfo *info) {
	set<Problem*>::const_iterator selection;
	Problem *solBT;

	{
		scoped_lock<decltype(mutex_population)> lock_population(mutex_population);

		executionCounter++;

		// Escolhe a melhor solucao para ser usada pelo BT
		if (parameters.choicePolicy == 0 || Random::randomPercentage() < (100 * parameters.elitismProbability)) {
			selection = sol->cbegin();
		} else {
			double fitTotal = parameters.choicePolicy < 0 ? Problem::sumFitnessMaximize(sol, sol->size()) : Problem::sumFitnessMaximize(sol, parameters.choicePolicy);

			selection = selectOpportunisticSolution(sol, fitTotal);
		}

		solBT = Problem::copySolution(**selection);
	}

	return exec(solBT, info);
}

set<Problem*>::const_iterator TabuSearch::selectOpportunisticSolution(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) {
	set<Problem*>::const_iterator selection = population->cbegin();
	int attemps = 0;

	while ((selection == population->cbegin() || (*selection)->heuristicsInfo.tabu == true) && (attemps++ < HEURISTIC_SELECTION_MAX_ATTEMPTS))
		selection = Problem::selectOpportunisticSolution(population, fitTotal);

	(*selection)->heuristicsInfo.tabu++;

	return selection;
}

void TabuSearch::markSolutions(vector<Problem*> *solutions) {
	for (auto solution = solutions->cbegin(); solution != solutions->cend(); ++solution) {
		(*solution)->heuristicsInfo.tabu++;
	}
}

vector<Problem*>* TabuSearch::exec(Problem *init, HeuristicExecutionInfo *info) {
	vector<pair<Problem*, InfoTabu*>*> *neighborhood;
	pair<Problem*, InfoTabu*> *local;

	// Lista Tabu de movimentos repetidos
	list<InfoTabu*> *listaTabu = new list<InfoTabu*>;

	// Maximos globais e locais na execucao da Busca Tabu
	vector<Problem*> *maxGlobal = new vector<Problem*>();
	Problem *maxLocal = init;

	maxGlobal->push_back(Problem::copySolution(*maxLocal));

	if (info != NULL) {
		info->bestInitialFitness = (*maxGlobal->begin())->getFitness();
		info->bestActualFitness = (*maxGlobal->rbegin())->getFitness();

		info->status = 0.0f;

		info->setupInfo("Iteration: %d", 0);
	}

	// Loop principal
	for (int i = 0, j = 0; i < parameters.iterations && j < parameters.attemptsWithoutImprovement; i++, j++) {
		if (STATUS != EXECUTING)
			break;

		if (info != NULL) {
			info->status = (100.00 * (double) (i + 1)) / (double) parameters.iterations;

			info->bestActualFitness = (*maxGlobal->rbegin())->getFitness();

			info->setupInfo("Iteration: %d", i + 1);
		}

		if (parameters.explorationPolicy <= 0 || parameters.explorationPolicy >= 1) {
			// Pega uma lista de todos os "vizinhos" de maxLocal
			neighborhood = maxLocal->localSearch();
		} else {
			// Pega uma parcela 'polExploracao' dos "vizinhos" de maxLocal
			neighborhood = maxLocal->localSearch(parameters.explorationPolicy);
		}

		// Escolhe a solucao de peso minimo
		while (!neighborhood->empty()) {
			local = neighborhood->back();
			neighborhood->pop_back();

			// Se nao for tabu...
			if (!isTabu(listaTabu, local->second)) {
				if (Problem::improvement(*maxLocal, *local->first) > 0)
					j = 0;

				delete maxLocal;
				maxLocal = local->first;

				if (Problem::improvement(*maxGlobal->back(), *local->first) > 0) {
					maxGlobal->push_back(Problem::copySolution(*maxLocal));
				}

				addTabu(listaTabu, local->second, parameters.listSize);

				delete local;

				break;
			}
			// Eh tabu...
			else {
				// Satisfaz a funcao de aspiracao
				if (aspiracao((double) parameters.aspirationCriteria, local->first, maxLocal, maxGlobal->back())) {
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
		while (!neighborhood->empty()) {
			local = neighborhood->back();
			neighborhood->pop_back();

			delete local->first;
			delete local->second;
			delete local;
		}
		neighborhood->clear();
		delete neighborhood;
	}
	delete maxLocal;

	while (!listaTabu->empty()) {
		delete listaTabu->back();
		listaTabu->pop_back();
	}
	listaTabu->clear();
	delete listaTabu;

	markSolutions(maxGlobal);

	if (info != NULL) {
		info->contribution = maxGlobal->size();
	}

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
