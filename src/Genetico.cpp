#include "Genetico.h"

using namespace std;

extern bool PARAR;

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

	vector<pair<Problema*, Problema*>* >::iterator iter1;
	vector<Problema*>::iterator iter2;

	vector<vector<pair<Problema*, Problema*>* >::iterator> paisIter;
	int paisSize, particao, mutacao;

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
				iter2 = Controle::selectRandom(bad_pop, rand());
				temp->first = *iter2;
			}
			else
			{
				iter2 = Controle::selectRouletteWheel(pop, sumP, rand());
				sumP -= (*iter2)->getFitnessMaximize();
				aux_pop->push_back(*iter2);
				temp->first = *iter2;
				pop->erase(iter2);
			}

			if(rand() < RAND_MAX*probMutacao && (int)bad_pop->size() > 0)
			{
				iter2 = Controle::selectRandom(bad_pop, rand());
				temp->second = *iter2;
			}
			else
			{
				iter2 = Controle::selectRouletteWheel(pop, sumP, rand());
				sumP -= (*iter2)->getFitnessMaximize();
				aux_pop->push_back(*iter2);
				temp->second = *iter2;
				pop->erase(iter2);
			}
			pais->push_back(temp);
		}

		/* OpenMP nao trabalha direito com STL */
		for(iter1 = pais->begin(); iter1 != pais->end(); iter1++)
			paisIter.push_back(iter1);
		paisSize = pais->size();
		particao = tamParticionamento;
		mutacao = probMutacao;

		/* Faz o cruzamento de todos os pares definidos anteriormente */
		#pragma omp parallel for shared(filhos, pais, paisIter, paisSize, particao, mutacao) private(i, temp, iter1)
		for(int i = 0; i < paisSize; i++)
		{
			iter1 = paisIter[i];

			if(i % 2 == 0)
			{
				/* Crossover com dois pontos de particionamento escolhidos aleatoriamente e tamanho 'tamParticionmento' */
				temp = (*iter1)->first->crossOver((*iter1)->second, particao);
			}
			else
			{
				/* Crossover com um ponto de particionamento escolhido aleatoriamente */
				temp = (*iter1)->first->crossOver((*iter1)->second);
			}

			if(rand() < ((RAND_MAX*mutacao)/2))
				temp->first->mutacao();

			if(rand() < ((RAND_MAX*mutacao)/2))
				temp->second->mutacao();

			#pragma omp critical
			{
				filhos->push_back(temp);
			}

			delete *iter1;
		}

		/* Restaura a populacao dos pais */
		for(iter2 = pop->begin(); iter2 != pop->end(); iter2++)
			aux_pop->push_back(*iter2);

		pop->clear();

		/* Adiciona a populacao dos filhos */
		for(iter1 = filhos->begin(); iter1 != filhos->end(); iter1++)
		{
			if((*iter1)->first->getFitnessMinimize() != -1)
				pop->push_back((*iter1)->first);
			else
				bad_pop->push_back((*iter1)->first);	// Armazenado para possivel reaproveitamento

			if((*iter1)->second->getFitnessMinimize() != -1)
				pop->push_back((*iter1)->second);
			else
				bad_pop->push_back((*iter1)->second);	// Armazenado para possivel reaproveitamento

			delete *iter1;
		}
		pais->clear();
		filhos->clear();

		/* Ordena a antiga populacao dos pais */
		sort(aux_pop->begin(), aux_pop->end(), fncomp2);

		/* Selecao dos melhores */
		for(iter2 = aux_pop->begin(); iter2 != aux_pop->end(); iter2++)
		{
			if((int)pop->size() < tamPopGenetico)
				pop->push_back(*iter2);
			else
				bad_pop->push_back(*iter2);	// Armazenado para possivel reaproveitamento
		}
		sort(pop->begin(), pop->end(), fncomp2);

		aux_pop->clear();
	}

	for(iter2 = bad_pop->begin(); iter2 != bad_pop->end(); iter2++)
		delete *iter2;

	bad_pop->clear();
	delete bad_pop;

	delete pais;
	delete filhos;
	delete aux_pop;

	return pop;
}
