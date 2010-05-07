#include "Controle.h"
#include "Genetico.h"

using namespace std;

extern bool PARAR;

pthread_mutex_t mutAG;

Genetico::Genetico()
{
	name = "DEFAULT_AG";
	prob = 40;
	polEscolha = -1;
	iterGenetico = 250;
	tamPopGenetico = 250;
	tamBadGenetico = 2500;
	probCrossOver = 0.9;
	probMutacao = 0.1;
	tamParticionamento = -1;

#ifndef THREADS
	bad_pop = new vector<Problema*>();
#endif

	Heuristica::numHeuristic += prob;
}

Genetico::Genetico(ParametrosAG* pAG)
{
	name = "AG";
	prob = pAG->probAG;
	polEscolha = pAG->polEscolha;
	iterGenetico = pAG->numeroIteracoes;
	tamPopGenetico = pAG->tamanhoPopulacao;
	tamBadGenetico = pAG->tamanhoAuxPopulacao != -1 ? pAG->tamanhoAuxPopulacao : 10*tamPopGenetico;
	probCrossOver = pAG->probCrossOver;
	probMutacao = pAG->probMutacao;
	tamParticionamento = pAG->tamanhoParticionamento;

#ifndef THREADS
	bad_pop = new vector<Problema*>();
#endif

	Heuristica::numHeuristic += prob;
}

Genetico::~Genetico()
{
	Heuristica::numHeuristic -= prob;

#ifndef THREADS
	for(vector<Problema*>::iterator iter = bad_pop->begin(); iter != bad_pop->end(); iter++)
		delete *iter;

	bad_pop->clear();
	delete bad_pop;
#endif
}

vector<Problema*>* Genetico::start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic)
{
	vector<Problema*>* popAG = new vector<Problema*>();
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter = sol->end();
	int i = 0;

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
		double visao = polEscolha < 0 ? Problema::totalMakespan : polEscolha;

		pthread_mutex_lock(&mutex);
		for(i = 1; i < tamPopGenetico; i++)
		{
			/* Evita a escolha de individuos repetidos */
			while(iter == sol->end() || find(popAG, *iter))
				iter = Controle::selectRouletteWheel(sol, (int)visao, rand());

			(*iter)->exec.genetico = true;
			popAG->push_back(Problema::alloc(**iter));
		}
		popAG->push_back(Problema::alloc(**sol->begin()));
		pthread_mutex_unlock(&mutex);

		sort(popAG->begin(), popAG->end(), fncomp2);
	}

	srand(unsigned(randomic));
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

	int numCrossOver;
	int sumP, sumB;

#ifdef THREADS
	vector<Problema*> *bad_pop = new vector<Problema*>();
#endif

	/* Iteracao principal do AG */
	for(int i = 0; i < iterGenetico; i++)
	{
		if(PARAR == true)
			break;

		numCrossOver = (int)((float)pop->size() * probCrossOver);

		/* Escolhe os casais de 'pop' que se cruzarao */
		sumP = (int)Problema::sumFitness(pop, pop->size());
		sumB = (int)Problema::sumFitness(bad_pop, bad_pop->size());

		for(int j = 0; j < numCrossOver/2; j++)
		{
			temp = new pair<Problema*, Problema*>();

			if(rand() < RAND_MAX*probMutacao && (int)bad_pop->size() > 0)
			{
				iter2 = Controle::selectRouletteWheel(bad_pop, sumB, rand());
				temp->first = *iter2;
			}
			else
			{
				iter2 = Controle::selectRouletteWheel(pop, sumP, rand());
				sumP -= (*iter2)->getFitness();
				aux_pop->push_back(*iter2);
				temp->first = *iter2;
				pop->erase(iter2);
			}

			if(rand() < RAND_MAX*probMutacao && (int)bad_pop->size() > 0)
			{
				iter2 = Controle::selectRouletteWheel(bad_pop, sumB, rand());
				temp->second = *iter2;
			}
			else
			{
				iter2 = Controle::selectRouletteWheel(pop, sumP, rand());
				sumP -= (*iter2)->getFitness();
				aux_pop->push_back(*iter2);
				temp->second = *iter2;
				pop->erase(iter2);
			}

			pais->push_back(temp);
		}

		/* Faz o cruzamento de todos os pares definidos anteriormente */
		for(iter1 = pais->begin(); iter1 != pais->end(); iter1++)
		{
			temp = (*iter1)->first->crossOver((*iter1)->second, tamParticionamento);

			if(rand() < (RAND_MAX*probMutacao/2))
				temp->first->mutacao();

			if(rand() < (RAND_MAX*probMutacao/2))
				temp->second->mutacao();

			filhos->push_back(temp);

			delete *iter1;
		}

		/* Restaura a populacao dos pais */
		for(iter2 = pop->begin(); iter2 != pop->end(); iter2++)
			aux_pop->push_back(*iter2);

		pop->clear();

		/* Adiciona a populacao dos filhos */
		for(iter1 = filhos->begin(); iter1 != filhos->end(); iter1++)
		{
			if((*iter1)->first->getMakespan() != -1)
				pop->push_back((*iter1)->first);
			else
				bad_pop->push_back((*iter1)->first);	// Armazenado para possivel reaproveitamento

			if((*iter1)->second->getMakespan() != -1)
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

	pop = isUnique(pop, tamPopGenetico);

#ifndef THREADS
	/* Ordena a polulacao ruim */
	sort(bad_pop->begin(), bad_pop->end(), fncomp2);
	bad_pop = isUnique(bad_pop, tamBadGenetico);
#else
	for(iter2 = bad_pop->begin(); iter2 != bad_pop->end(); iter2++)
		delete *iter2;

	bad_pop->clear();
	delete bad_pop;
#endif

	delete pais;
	delete filhos;

	return pop;
}

inline vector<Problema*>* isUnique(vector<Problema*>* pop, int n)
{
	vector<Problema*>* aux = new vector<Problema*>();
	int max = 0;

	for(max = 1; max < (int)pop->size(); max++)
	{
		if(fnequal(pop->at(max-1), pop->at(max)) || max >= n)
			delete pop->at(max-1);
		else
			aux->push_back(pop->at(max-1));
	}
	if(max >= n)
		delete pop->at(max-1);
	else
		aux->push_back(pop->at(max-1));

	pop->clear();
	delete pop;

	return aux;
}

inline bool find(vector<Problema*> *vect, Problema *p)
{
	vector<Problema*>::iterator iter;

	for(iter = vect->begin(); iter != vect->end(); iter++)
		if(fnequal((*iter), p) == true)
			return true;

	return false;
}
