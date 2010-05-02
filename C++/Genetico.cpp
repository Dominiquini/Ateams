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
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	int i = 0;

	if(polEscolha == 0)
	{
		for(iter = sol->begin(), i = 0; iter != sol->end() && i < tamPopGenetico; iter++, i++)
		{
			(*iter)->exec.genetico = true;
			popAG->push_back(Problema::alloc(**iter));
		}
	}
	else
	{
		int visao = polEscolha < 0 ? Problema::totalMakespan : polEscolha;
		for(i = 1; i < tamPopGenetico; i++)
		{
			iter = Controle::selectRouletteWheel(sol, visao, rand());

			(*iter)->exec.genetico = true;
			popAG->push_back(Problema::alloc(**iter));
		}
		popAG->push_back(Problema::alloc(**sol->begin()));

		sort(popAG->begin(), popAG->end(), fncomp2);
	}

	srand(unsigned(randomic));
	return exec(popAG);
}

vector<Problema*>* Genetico::exec(vector<Problema*>* pop)
{
	Problema* bestLocal;
	pair<Problema*, Problema*>* temp;
	vector<Problema*>* aux_pop = new vector<Problema*>();
	vector<Problema*>* bad_pop = new vector<Problema*>();
	vector<pair<Problema*, Problema*>* > *pais, *filhos;
	pais = new vector<pair<Problema*, Problema*>* >();
	filhos = new vector<pair<Problema*, Problema*>* >();

	vector<pair<Problema*, Problema*>* >::iterator iter1;
	vector<Problema*>::iterator iter2;

	int numCrossOver = tamPopGenetico * probCrossOver;
	int sumP, sumB;

	bad_pop->push_back(Problema::alloc(*pop->front()));

	/* Iteracao principal do AG */
	for(int i = 0; i < iterGenetico; i++)
	{
		if(PARAR == 1)
			break;

		/* Salva a melhor solucao atual */
		bestLocal = Problema::alloc(*pop->front());

		/* Escolhe os casais de 'pop' que se cruzarao */
		sumP = Problema::sumFitness(pop, pop->size());
		sumB = Problema::sumFitness(bad_pop, bad_pop->size());

		temp = new pair<Problema*, Problema*>();
		temp->first = bestLocal;					// Garante a inclusao da melhor solucao atual
		temp->second = Controle::selectRouletteWheel(pop, sumP, rand());

		pais->push_back(temp);
		for(int j = 1; j < numCrossOver; j++)
		{
			temp = new pair<Problema*, Problema*>();

			if(rand() < RAND_MAX*probMutacao)
				temp->first = Controle::selectRouletteWheel(bad_pop, sumB, rand());
			else
				temp->first = Controle::selectRouletteWheel(pop, sumP, rand());

			if(rand() < RAND_MAX*probMutacao)
				temp->second = Controle::selectRouletteWheel(bad_pop, sumB, rand());
			else
				temp->second = Controle::selectRouletteWheel(pop, sumP, rand());

			pais->push_back(temp);
		}

		/* Faz o cruzamento de todos os pares definidos anteriormente */
		for(iter1 = pais->begin(); iter1 != pais->end(); iter1++)
		{
			temp = (*iter1)->first->crossOver((*iter1)->second, tamParticionamento);

			filhos->push_back(temp);

			delete *iter1;
		}

		/* Remove populacao dos pais */
		for(iter2 = pop->begin(); iter2 != pop->end(); iter2++)
			delete *iter2;
		pop->clear();

		/* Adiciona a populacao dos filhos, garantindo a presenca da melhor solucao */
		aux_pop->push_back(bestLocal);
		for(iter1 = filhos->begin(); iter1 != filhos->end(); iter1++)
		{
			aux_pop->push_back((*iter1)->first);
			aux_pop->push_back((*iter1)->second);

			delete *iter1;
		}

		pais->clear();
		filhos->clear();

		/* Ordena a nova populacao */
		sort(aux_pop->begin(), aux_pop->end(), fncomp2);

		/* Selecao dos melhores */
		for(iter2 = aux_pop->begin(); iter2 != aux_pop->end(); iter2++)
		{
			/* Introduz uma mutacao com chance 'probMutacao' */
			if(rand() < RAND_MAX*probMutacao)
				(*iter2)->mutacao();

			if((*iter2)->getMakespan() != -1 && (int)pop->size() < tamPopGenetico)
				pop->push_back(*iter2);		// Colocado no conjunto de boas solucoes
			else
				bad_pop->push_back(*iter2);	// Armazenado para possivel reaproveitamento
		}

		aux_pop->clear();
	}
	/* Remove populacao dos pais */
	for(iter2 = bad_pop->begin(); iter2 != bad_pop->end(); iter2++)
		delete *iter2;
	bad_pop->clear();

	delete pais;
	delete filhos;
	delete aux_pop;
	delete bad_pop;

	return pop;
}
