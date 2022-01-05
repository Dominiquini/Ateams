#include "HeuristicGenetic.hpp"

using namespace std;

GeneticAlgorithm::GeneticAlgorithm() : Heuristic::Heuristic() {
}

GeneticAlgorithm::~GeneticAlgorithm() {
}

HeuristicParameters GeneticAlgorithm::getParameters() {
	return parameters;
}

bool GeneticAlgorithm::setParameter(const char *parameter, const char *value) {
	return parameters.setParameter(parameter, value);
}

vector<Problem*>* GeneticAlgorithm::start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicExecutionInfo *listener) {
	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator selection = sol->begin();
	vector<Problem*> *popAG = new vector<Problem*>();
	int initialPopulation = 0;
	int solutionsCount = 0;

	pthread_mutex_lock(&mutex_pop);

	initialPopulation = min(parameters.populationSize, (int) sol->size());

	executionCounter++;

	if (parameters.choicePolicy == 0) {
		for (selection = sol->begin(), solutionsCount = 0; selection != sol->end() && solutionsCount < initialPopulation; selection++, solutionsCount++) {
			popAG->push_back(Problem::copySolution(**selection));
		}
	} else {
		double fitTotal = parameters.choicePolicy < 0 ? Control::sumFitnessMaximize(sol, sol->size()) : Control::sumFitnessMaximize(sol, parameters.choicePolicy);

		for (solutionsCount = 0; solutionsCount < initialPopulation; solutionsCount++) {
			selection = selectRouletteWheel(sol, fitTotal);

			popAG->push_back(Problem::copySolution(**selection));
		}
	}

	sort(popAG->begin(), popAG->end(), fnSortFitness);

	pthread_mutex_unlock(&mutex_pop);

	return exec(popAG, listener);
}

set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator GeneticAlgorithm::selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) {
	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator selection = population->begin();
	int attemps = 0;

	while ((selection == population->begin() || (*selection)->heuristicsInfo.genetic == true) && (attemps++ < MAX_ATTEMPTS))
		selection = Control::selectRouletteWheel(population, fitTotal);

	(*selection)->heuristicsInfo.genetic++;

	return selection;
}

void GeneticAlgorithm::markSolutions(vector<Problem*> *solutions) {
	for (auto solution = solutions->cbegin(); solution != solutions->cend(); ++solution) {
		(*solution)->heuristicsInfo.genetic++;
	}
}

vector<Problem*>* GeneticAlgorithm::exec(vector<Problem*> *pop, HeuristicExecutionInfo *listener) {
	Problem *mutante;
	pair<Problem*, Problem*> *temp;
	vector<Problem*> *aux_pop = new vector<Problem*>();
	vector<pair<Problem*, Problem*>*> *pais, *filhos;
	pais = new vector<pair<Problem*, Problem*>*>();
	filhos = new vector<pair<Problem*, Problem*>*>();

	vector<pair<Problem*, Problem*>*>::const_iterator iterParProb;
	vector<Problem*>::iterator iterProb;

	int strengthCrossOver = (int) (parameters.crossoverPower * 100);
	int numCrossOver;
	double sumP;

	vector<Problem*> *bad_pop = new vector<Problem*>();

	if (listener != NULL)
		listener->bestInitialFitness = (*pop->begin())->getFitness();

	/* Iteracao principal do AG */
	for (int i = 0; i < parameters.iterations; i++) {
		if (STATUS != EXECUTING)
			break;

		if (listener != NULL) {
			listener->status = (100.00 * (double) (i + 1)) / (double) parameters.iterations;

			listener->bestActualFitness = (*pop->begin())->getFitness();

			listener->setupInfo("Generation: %d", i + 1);
		}

		numCrossOver = (int) ((float) pop->size() * parameters.crossoverProbability);

		sumP = Control::sumFitnessMaximize(pop, pop->size());

		/* Escolhe os casais de 'pop' que se cruzarao */
		for (int j = 0; j < numCrossOver / 2; j++) {
			temp = new pair<Problem*, Problem*>();

			if (random() < RAND_MAX * parameters.mutationProbability && (int) bad_pop->size() > 0) {
				iterProb = Control::selectRandom(bad_pop);
				temp->first = *iterProb;
			} else {
				iterProb = Control::selectRouletteWheel(pop, sumP);
				sumP -= (*iterProb)->getFitnessMaximize();
				aux_pop->push_back(*iterProb);
				temp->first = *iterProb;
				pop->erase(iterProb);
			}

			if (random() < RAND_MAX * parameters.mutationProbability && (int) bad_pop->size() > 0) {
				iterProb = Control::selectRandom(bad_pop);
				temp->second = *iterProb;
			} else {
				iterProb = Control::selectRouletteWheel(pop, sumP);
				sumP -= (*iterProb)->getFitnessMaximize();
				aux_pop->push_back(*iterProb);
				temp->second = *iterProb;
				pop->erase(iterProb);
			}
			pais->push_back(temp);
		}

		/* Faz o cruzamento entre os individuos anteriormente escolhidos */
		for (iterParProb = pais->begin(); iterParProb != pais->end(); iterParProb++) {
			if (parameters.partitionSize != -1) {
				/* Crossover com dois pontos de particionamento escolhidos aleatoriamente e um deles com tamanho 'tamParticionmento' */
				temp = (*iterParProb)->first->crossOver((*iterParProb)->second, parameters.partitionSize, strengthCrossOver);
			} else {
				/* Crossover com um ponto de particionamento escolhido aleatoriamente */
				temp = (*iterParProb)->first->crossOver((*iterParProb)->second, strengthCrossOver);
			}

			if (random() < (RAND_MAX * parameters.mutationProbability / 2)) {
				mutante = temp->first->mutation(random(1, (int) (((float) 100) * parameters.mutationProbability)));
				delete temp->first;
				temp->first = mutante;
			}

			if (random() < (RAND_MAX * parameters.mutationProbability / 2)) {
				mutante = temp->second->mutation(random(1, (int) (((float) 100) * parameters.mutationProbability)));
				delete temp->second;
				temp->second = mutante;
			}

			filhos->push_back(temp);

			delete *iterParProb;
		}

		/* Restaura a populacao dos pais */
		for (iterProb = pop->begin(); iterProb != pop->end(); iterProb++)
			aux_pop->push_back(*iterProb);

		pop->clear();

		/* Adiciona a populacao dos filhos */
		for (iterParProb = filhos->begin(); iterParProb != filhos->end(); iterParProb++) {
			if ((*iterParProb)->first->getFitness() != -1)
				pop->push_back((*iterParProb)->first);
			else
				bad_pop->push_back((*iterParProb)->first);	// Armazenado para possivel reaproveitamento

			if ((*iterParProb)->second->getFitness() != -1)
				pop->push_back((*iterParProb)->second);
			else
				bad_pop->push_back((*iterParProb)->second);	// Armazenado para possivel reaproveitamento

			delete *iterParProb;
		}
		pais->clear();
		filhos->clear();

		/* Ordena a antiga populacao dos pais */
		sort(aux_pop->begin(), aux_pop->end(), fnSortFitness);

		/* Selecao dos melhores */
		for (iterProb = aux_pop->begin(); iterProb != aux_pop->end(); iterProb++) {
			if ((int) pop->size() < parameters.populationSize)
				pop->push_back(*iterProb);
			else
				bad_pop->push_back(*iterProb);	// Armazenado para possivel reaproveitamento
		}

		aux_pop->clear();

		sort(pop->begin(), pop->end(), fnSortFitness);

		/* Mantem a populacao auxiliar sob controle */
		random_shuffle(bad_pop->begin(), bad_pop->end(), pointer_to_unary_function<int, int>(random));

		while ((int) bad_pop->size() > 10 * parameters.populationSize) {
			delete bad_pop->back();
			bad_pop->pop_back();
		}
	}

	for (iterProb = bad_pop->begin(); iterProb != bad_pop->end(); iterProb++)
		delete *iterProb;

	bad_pop->clear();
	delete bad_pop;

	delete pais;
	delete filhos;
	delete aux_pop;

	markSolutions(pop);

	return pop;
}
