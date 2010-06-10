#include "Genetico.h"

using namespace std;

Genetico::Genetico() : Heuristica::Heuristica("DEFAULT_AG")
{
	numExec = 0;

	prob = 20;
	polEscolha = -1;
	iterGenetico = 500;
	tamPopGenetico = 250;
	probCrossOver = 0.8;
	probMutacao = 0.02;
	tamParticionamento = -1;

	Heuristica::numHeuristic += prob;
}

Genetico::Genetico(string nome, ParametrosHeuristicas& pAG) : Heuristica::Heuristica(nome)
{
	numExec = 0;

	prob = pAG.probAG != -1 ? pAG.probAG : 20;
	polEscolha = pAG.polEscolhaAG != -1 ? pAG.polEscolhaAG : -1;
	iterGenetico = pAG.iterAG != -1 ? pAG.iterAG : 500;
	tamPopGenetico = pAG.tamPopAG != -1 ? pAG.tamPopAG : 250;
	probCrossOver = pAG.probCrossOverAG != -1 ? pAG.probCrossOverAG : 0.8;
	probMutacao = pAG.probMutacaoAG != -1 ? pAG.probMutacaoAG : 0.02;
	tamParticionamento = pAG.tamParticaoAG != -1 ? pAG.tamParticaoAG : -1;

	Heuristica::numHeuristic += prob;
}

Genetico::~Genetico()
{
	Heuristica::numHeuristic -= prob;
}

vector<Problema*>* Genetico::start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic)
{
	vector<Problema*>* popAG = new vector<Problema*>();
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter = sol->end();
	int i = 0;

	numExec++;

	if(polEscolha == 0)
	{
		pthread_mutex_lock(&mutex);
		for(iter = sol->begin(), i = 0; iter != sol->end() && i < tamPopGenetico; iter++, i++)
		{
			(*iter)->exec.genetico = true;
			popAG->push_back(Problema::alloc(**iter));
		}
		pthread_mutex_unlock(&mutex);
	}
	else
	{
		double visao = polEscolha < 0 ? Problema::sumFitnessMaximize(sol, sol->size()) : Problema::sumFitnessMaximize(sol, polEscolha);

		srand(randomic);

		pthread_mutex_lock(&mutex);
		for(i = 1; i < tamPopGenetico; i++)
		{
			/* Evita a escolha de individuos repetidos */
			while(iter == sol->end() || (*iter)->exec.genetico == true)
				iter = Controle::selectRouletteWheel(sol, visao, rand());

			(*iter)->exec.genetico = true;
			popAG->push_back(Problema::alloc(**iter));
		}
		popAG->push_back(Problema::alloc(**sol->begin()));

		for(iter = sol->begin(); iter != sol->end(); iter++)
			(*iter)->exec.genetico = false;

		pthread_mutex_unlock(&mutex);

		sort(popAG->begin(), popAG->end(), fncomp2);
	}
	return exec(popAG);
}

vector<Problema*>* Genetico::exec(vector<Problema*>* pop)
{
	pair<Problema*, Problema*>* temp;
	vector<Problema*> *aux_pop = new vector<Problema*>();
	vector<pair<Problema*, Problema*>* > *pais, *filhos;
	pais = new vector<pair<Problema*, Problema*>* >();
	filhos = new vector<pair<Problema*, Problema*>* >();

	vector<pair<Problema*, Problema*>* >::iterator iterParProb;
	vector<Problema*>::iterator iterProb;

	int numCrossOver;
	double sumP;

	vector<Problema*> *bad_pop = new vector<Problema*>();

	/* Iteracao principal do AG */
	for(int i = 0; i < iterGenetico; i++)
	{
		if(PARAR == true)
			break;

		numCrossOver = (int)((float)pop->size() * probCrossOver);

		/* Escolhe os casais de 'pop' que se cruzarao */
		sumP = (int)Problema::sumFitnessMaximize(pop, pop->size());

		for(int j = 0; j < numCrossOver/2; j++)
		{
			temp = new pair<Problema*, Problema*>();

			if(rand() < RAND_MAX*probMutacao && (int)bad_pop->size() > 0)
			{
				iterProb = Controle::selectRandom(bad_pop, rand());
				temp->first = *iterProb;
			}
			else
			{
				iterProb = Controle::selectRouletteWheel(pop, sumP, rand());
				sumP -= (*iterProb)->getFitnessMaximize();
				aux_pop->push_back(*iterProb);
				temp->first = *iterProb;
				pop->erase(iterProb);
			}

			if(rand() < RAND_MAX*probMutacao && (int)bad_pop->size() > 0)
			{
				iterProb = Controle::selectRandom(bad_pop, rand());
				temp->second = *iterProb;
			}
			else
			{
				iterProb = Controle::selectRouletteWheel(pop, sumP, rand());
				sumP -= (*iterProb)->getFitnessMaximize();
				aux_pop->push_back(*iterProb);
				temp->second = *iterProb;
				pop->erase(iterProb);
			}
			pais->push_back(temp);
		}

		for(iterParProb = pais->begin(); iterParProb != pais->end(); iterParProb++)
		{
			if(i % 2 == 0)
			{
				/* Crossover com dois pontos de particionamento escolhidos aleatoriamente e tamanho 'tamParticionmento' */
				temp = (*iterParProb)->first->crossOver((*iterParProb)->second, tamParticionamento);
			}
			else
			{
				/* Crossover com um ponto de particionamento escolhido aleatoriamente */
				temp = (*iterParProb)->first->crossOver((*iterParProb)->second);
			}

			if(rand() < (RAND_MAX*probMutacao/2))
				temp->first->mutacao();

			if(rand() < (RAND_MAX*probMutacao/2))
				temp->second->mutacao();

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
			if((*iterParProb)->first->getFitnessMinimize() != -1)
				pop->push_back((*iterParProb)->first);
			else
				bad_pop->push_back((*iterParProb)->first);	// Armazenado para possivel reaproveitamento

			if((*iterParProb)->second->getFitnessMinimize() != -1)
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
