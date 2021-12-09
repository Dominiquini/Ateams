#include "Genetico.hpp"

using namespace std;

Genetico::Genetico() : Heuristica::Heuristica("DEFAULT_AG")
{
	numExec = 0;

	prob = 13;
	polEscolha = -1;
	iterGenetico = 500;
	tamPopGenetico = 250;
	probCrossOver = 0.8;
	powerCrossOver = 0.5;
	probMutacao = 0.08;
	tamParticionamento = 0;

	Heuristica::numHeuristic += prob;
}

Genetico::~Genetico()
{
	Heuristica::numHeuristic -= prob;
}


bool Genetico::setParameter(const char* parameter, const char* value)
{
	if(Heuristica::setParameter(parameter, value))
		return true;

	if(strcasecmp(parameter, "probAG") == 0)
	{
		Heuristica::numHeuristic -= prob;
		sscanf(value, "%d", &prob);
		Heuristica::numHeuristic += prob;
	}
	else if(strcasecmp(parameter, "polEscolhaAG") == 0)
	{
		sscanf(value, "%d", &polEscolha);
	}
	else if(strcasecmp(parameter, "iterAG") == 0)
	{
		sscanf(value, "%d", &iterGenetico);
	}
	else if(strcasecmp(parameter, "tamPopAG") == 0)
	{
		sscanf(value, "%d", &tamPopGenetico);
	}
	else if(strcasecmp(parameter, "tamParticaoAG") == 0)
	{
		sscanf(value, "%d", &tamParticionamento);
	}
	else if(strcasecmp(parameter, "probCrossOverAG") == 0)
	{
		sscanf(value, "%f", &probCrossOver);
	}
	else if(strcasecmp(parameter, "powerCrossOver") == 0)
	{
		sscanf(value, "%f", &powerCrossOver);
	}
	else if(strcasecmp(parameter, "probMutacaoAG") == 0)
	{
		sscanf(value, "%f", &probMutacao);
	}
	else
	{
		return false;
	}

	return true;
}

vector<Problem*>* Genetico::start(set<Problem*, bool(*)(Problem*, Problem*)>* sol, Heuristica_Listener* listener)
{
	vector<Problem*>* popAG = new vector<Problem*>();
	set<Problem*, bool(*)(Problem*, Problem*)>::const_iterator iter = sol->end();
	int i = 0, j = 0;

	if(tamPopGenetico > (int)sol->size())
		tamPopGenetico = (int)sol->size();

	numExec++;

	pthread_mutex_lock(&mutex_pop);

	if(polEscolha == 0)
	{
		for(iter = sol->begin(), i = 0; iter != sol->end() && i < tamPopGenetico; iter++, i++)
		{
			(*iter)->exec.genetico = true;
			popAG->push_back(Problem::copySoluction(**iter));
		}
	}
	else
	{
		double visao = polEscolha < 0 ? Control::sumFitnessMaximize(sol, sol->size()) : Control::sumFitnessMaximize(sol, polEscolha);

		for(i = 1; i < tamPopGenetico; i++)
		{
			/* Evita a escolha de individuos repetidos */
			while((iter == sol->end() || (*iter)->exec.genetico == true) && (j++ < tamPopGenetico))
				iter = Control::selectRouletteWheel(sol, visao);

			j= 0;
			(*iter)->exec.genetico = true;
			popAG->push_back(Problem::copySoluction(**iter));
		}
		popAG->push_back(Problem::copySoluction(**sol->begin()));

		for(iter = sol->begin(); iter != sol->end(); iter++)
			(*iter)->exec.genetico = false;

		sort(popAG->begin(), popAG->end(), fncomp2);
	}

	pthread_mutex_unlock(&mutex_pop);

	return exec(popAG, listener);
}

vector<Problem*>* Genetico::exec(vector<Problem*>* pop, Heuristica_Listener* listener)
{
	Problem *mutante;
	pair<Problem*, Problem*>* temp;
	vector<Problem*> *aux_pop = new vector<Problem*>();
	vector<pair<Problem*, Problem*>* > *pais, *filhos;
	pais = new vector<pair<Problem*, Problem*>* >();
	filhos = new vector<pair<Problem*, Problem*>* >();

	vector<pair<Problem*, Problem*>* >::const_iterator iterParProb;
	vector<Problem*>::iterator iterProb;

	int strengthCrossOver = (int)(powerCrossOver * 100);
	int numCrossOver;
	double sumP;

	vector<Problem*> *bad_pop = new vector<Problem*>();

	if(listener != NULL)
		listener->bestInitialFitness = (*pop->begin())->getFitness();

	/* Iteracao principal do AG */
	for(int i = 0; i < iterGenetico; i++)
	{
		if(TERMINATE == true)
			break;

		if(listener != NULL)
		{
			listener->status = (100.00 * (double)(i+1)) / (double)iterGenetico;

			listener->bestActualFitness = (*pop->begin())->getFitness();

			listener->setInfo("Generation: %d", i+1);
		}

		numCrossOver = (int)((float)pop->size() * probCrossOver);

		sumP = Control::sumFitnessMaximize(pop, pop->size());

		/* Escolhe os casais de 'pop' que se cruzarao */
		for(int j = 0; j < numCrossOver/2; j++)
		{
			temp = new pair<Problem*, Problem*>();

			if(xRand() < RAND_MAX*probMutacao && (int)bad_pop->size() > 0)
			{
				iterProb = Control::selectRandom(bad_pop);
				temp->first = *iterProb;
			}
			else
			{
				iterProb = Control::selectRouletteWheel(pop, sumP);
				sumP -= (*iterProb)->getFitnessMaximize();
				aux_pop->push_back(*iterProb);
				temp->first = *iterProb;
				pop->erase(iterProb);
			}

			if(xRand() < RAND_MAX*probMutacao && (int)bad_pop->size() > 0)
			{
				iterProb = Control::selectRandom(bad_pop);
				temp->second = *iterProb;
			}
			else
			{
				iterProb = Control::selectRouletteWheel(pop, sumP);
				sumP -= (*iterProb)->getFitnessMaximize();
				aux_pop->push_back(*iterProb);
				temp->second = *iterProb;
				pop->erase(iterProb);
			}
			pais->push_back(temp);
		}

		/* Faz o cruzamento entre os individuos anteriormente escolhidos */
		for(iterParProb = pais->begin(); iterParProb != pais->end(); iterParProb++)
		{
			if(tamParticionamento != -1)
			{
				/* Crossover com dois pontos de particionamento escolhidos aleatoriamente e um deles com tamanho 'tamParticionmento' */
				temp = (*iterParProb)->first->crossOver((*iterParProb)->second, tamParticionamento, strengthCrossOver);
			}
			else
			{
				/* Crossover com um ponto de particionamento escolhido aleatoriamente */
				temp = (*iterParProb)->first->crossOver((*iterParProb)->second, strengthCrossOver);
			}

			if(xRand() < (RAND_MAX*probMutacao/2))
			{
				mutante = temp->first->mutacao(xRand(1, (int)(((float)100)*probMutacao)));
				delete temp->first;
				temp->first = mutante;
			}

			if(xRand() < (RAND_MAX*probMutacao/2))
			{
				mutante = temp->second->mutacao(xRand(1, (int)(((float)100)*probMutacao)));
				delete temp->second;
				temp->second = mutante;
			}

			filhos->push_back(temp);

			delete *iterParProb;
		}

		/* Restaura a populacao dos pais */
		for(iterProb = pop->begin(); iterProb != pop->end(); iterProb++)
			aux_pop->push_back(*iterProb);

		pop->clear();

		/* Adiciona a populacao dos filhos */
		for(iterParProb = filhos->begin(); iterParProb != filhos->end(); iterParProb++)
		{
			if((*iterParProb)->first->getFitness() != -1)
				pop->push_back((*iterParProb)->first);
			else
				bad_pop->push_back((*iterParProb)->first);	// Armazenado para possivel reaproveitamento

			if((*iterParProb)->second->getFitness() != -1)
				pop->push_back((*iterParProb)->second);
			else
				bad_pop->push_back((*iterParProb)->second);	// Armazenado para possivel reaproveitamento

			delete *iterParProb;
		}
		pais->clear();
		filhos->clear();

		/* Ordena a antiga populacao dos pais */
		sort(aux_pop->begin(), aux_pop->end(), fncomp2);

		/* Selecao dos melhores */
		for(iterProb = aux_pop->begin(); iterProb != aux_pop->end(); iterProb++)
		{
			if((int)pop->size() < tamPopGenetico)
				pop->push_back(*iterProb);
			else
				bad_pop->push_back(*iterProb);	// Armazenado para possivel reaproveitamento
		}
		sort(pop->begin(), pop->end(), fncomp2);

		/* Mantem a populacao auxiliar sob controle */
		random_shuffle(bad_pop->begin(), bad_pop->end());
		while((int)bad_pop->size() > 10*tamPopGenetico)
		{
			delete bad_pop->back();
			bad_pop->pop_back();
		}

		aux_pop->clear();
	}

	for(iterProb = bad_pop->begin(); iterProb != bad_pop->end(); iterProb++)
		delete *iterProb;

	bad_pop->clear();
	delete bad_pop;

	delete pais;
	delete filhos;
	delete aux_pop;

	return pop;
}
