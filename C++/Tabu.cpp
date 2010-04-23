#include "Tabu.h"

using namespace std;

Tabu::Tabu(int iter, int lista)
{
	iterTabu = iter;
	tamListaTabu = lista;
}

/* Executa uma busca por soluções a partir de 'init' por 'iterTabu' vezes */
Problema* Tabu::start(Problema* init)
{
	multiset<Problema*, bool(*)(Problema*, Problema*)>* local;
	multiset<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;

	list<mov> *listaTabu = new list<mov>;

	Problema *maxGlobal = new JobShop(*init), *maxLocal = new JobShop(*init);

	// Loop principal
	for(int i = 0; i < iterTabu; i++)
	{
		local = maxLocal->buscaLocal();

		// Pega a primeira solucao nao tabu
		for(iter = local->begin(); iter != local->end(); iter++)
		{
			// Se nao for tabu...
			if(!isTabu(listaTabu, (*iter)->movTabu))
			{
				delete maxLocal;
				maxLocal = new JobShop(**iter);
				if((*iter)->makespan < maxGlobal->makespan)
				{
					delete maxGlobal;
					maxGlobal = new JobShop(*maxLocal);
				}
				listaTabu->push_front((*iter)->movTabu);
				if((int)listaTabu->size() > tamListaTabu)
					listaTabu->pop_back();

				break;
			}
			// Eh tabu...
			else
			{
				// Satisfaz a funcao de aspiracao
				if((*iter)->makespan < maxGlobal->makespan)
				{
					delete maxLocal;
					maxLocal = new JobShop(**iter);

					delete maxGlobal;
					maxGlobal = new JobShop(*maxLocal);

					break;
				}
			}
		}

		for(iter = local->begin(); iter != local->end(); iter++)
			delete *iter;
		local->clear();
		delete local;
	}
	delete listaTabu;
	delete maxLocal;
	return maxGlobal;
}

/* Verdadeiro se solucao avaliada for Tabu */
bool isTabu(list<mov> *listaTabu, mov m)
{
	list<mov>::iterator iter;

	for(iter = listaTabu->begin(); iter != listaTabu->end(); iter++)
		if(iter->maq == m.maq && (iter->A == m.A || iter->A == m.B) && (iter->B == m.B || iter->B == m.A))
			return true;

	return false;
}
