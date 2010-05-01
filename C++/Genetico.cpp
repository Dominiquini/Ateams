#include "Controle.h"
#include "Genetico.h"

using namespace std;

extern int PARAR;

Genetico::Genetico()
{
	name = "DEFAULT_AG";
	prob = 50;
	polEscolha = -1;
	iterGenetico = 100;
	tamPopGenetico = 250;
	probCrossOver = 0.8;
	probMutacao = 0.1;
	tamParticionamento = -1;

	Heuristica::numHeuristic += prob;
}

Genetico::Genetico(ParametrosAG* pAG)
{
	name = "AG";
	prob = pAG->probAG;
	polEscolha = pAG->polEscolha;
	iterGenetico = pAG->numeroIteracoes;
	tamPopGenetico = pAG->tamanhoPopulacao;
	probCrossOver = pAG->probCrossOver;
	probMutacao = pAG->probMutacao;
	tamParticionamento = pAG->tamanhoParticionamento;

	Heuristica::numHeuristic += prob;
}

vector<Problema*>* Genetico::start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic)
{
	vector<Problema*>* popAG = new vector<Problema*>();

	int i = 0;
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	for(iter = sol->begin(), i = 0; iter != sol->end() && i < tamPopGenetico; iter++, i++)
	{
		(*iter)->exec.genetico = true;
		popAG->push_back(Problema::alloc(**iter));
	}

	srand(unsigned(randomic));
	return exec(popAG);
}

vector<Problema*>* Genetico::exec(vector<Problema*>* pop)
{
	Problema* bestLocal;
	pair<Problema*, Problema*>* temp;
	vector<Problema*>* aux_pop = new vector<Problema*>();
	vector<pair<Problema*, Problema*>* > *pais, *filhos;
	pais = new vector<pair<Problema*, Problema*>* >();
	filhos = new vector<pair<Problema*, Problema*>* >();

	vector<pair<Problema*, Problema*>* >::iterator iter1;
	vector<Problema*>::iterator iter2;

	int numCrossOver = tamPopGenetico * probCrossOver;
	int sum;

	/* Iteracao principal do AG */
	for(int i = 0; i < iterGenetico; i++)
	{
		if(PARAR == 1)
			break;

		/* Salva a melhor solucao atual */
		bestLocal = Problema::alloc(*pop->front());

		/* Escolhe os casais de 'pop' que se cruzarao */
		sum = Problema::sumFitness(pop, pop->size());

		temp = new pair<Problema*, Problema*>();
		temp->first = bestLocal;					// Garante a inclusao da melhor solucao atual
		temp->second = Controle::selectRouletteWheel(pop, sum, rand());

		pais->push_back(temp);
		for(int j = 1; j < numCrossOver; j++)
		{
			temp = new pair<Problema*, Problema*>();
			temp->first = Controle::selectRouletteWheel(pop, sum, rand());
			temp->second = Controle::selectRouletteWheel(pop, sum, rand());

			pais->push_back(temp);
		}

		/* Faz o cruzamento de todos os pares definidos anteriormente */
		for(iter1 = pais->begin(); iter1 != pais->end(); iter1++)
		{
			temp = (*iter1)->first->crossOver((*iter1)->second, tamParticionamento);
/*
			cout << "\n\nPai1: " << (*iter1)->first->getMakespan() << endl;
			(*iter1)->first->imprimir(false);
			cout << "\n\nPai2: " << (*iter1)->second->getMakespan() << endl;
			(*iter1)->second->imprimir(false);

			cout << "\n\nFil1: " << temp->first->getMakespan() << endl;
			temp->first->imprimir(false);
			cout << "\n\nFil2: " << temp->second->getMakespan() << endl;
			temp->second->imprimir(false);
*/
			filhos->push_back(temp);
		}

		/* Remove populacao dos pais */
		for(iter2 = pop->begin(); iter2 != pop->end(); iter2++)
			delete *iter2;
		pop->clear();

		/* Adiciona a populacao dos filhos */
		aux_pop->push_back(bestLocal);
		for(iter1 = filhos->begin(); iter1 != filhos->end(); iter1++)
		{
			aux_pop->push_back((*iter1)->first);
			aux_pop->push_back((*iter1)->second);
		}

		pais->clear();
		filhos->clear();

		/* Ordena a nova populacao */
		sort(aux_pop->begin(), aux_pop->end(), fncomp2);

		/* Selecao dos melhores */
		iter2 = aux_pop->begin();
		for(int j = 0; iter2 != aux_pop->end(); iter2++, j++)
			if(j < tamPopGenetico && (*iter2)->getMakespan() != -1)
				pop->push_back(*iter2);
			else
				delete *iter2;

		aux_pop->clear();
	}

	return pop;
}
