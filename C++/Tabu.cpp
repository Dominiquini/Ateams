#include "Controle.h"
#include "Tabu.h"

using namespace std;

Tabu::Tabu(ParametrosBT* pBT)
{
	name = "BT";
	prob = pBT->probBT;
	iterTabu = pBT->numeroIteracoes;
	tamListaTabu = pBT->tamanhoListaTabu;
	tentSemMelhora = pBT->tentativasSemMelhora;

	Heuristica::numHeuristic += prob;
}

/* Executa uma Busca Tabu na população com o devido criterio de selecao */
vector<Problema*>* Tabu::start(multiset<Problema*, bool(*)(Problema*, Problema*)>* sol)
{
	Problema *select = NULL;

	// Evita trabalhar sobre solucoes ja selecionadas anteriormente
	for(int i = 0; i < (int)sol->size()/10 && (select == NULL || select->movTabu.job == true); i++)
		select = Controle::selectRouletteWheel(sol, Problema::totalMakespan);
	select->movTabu.job = true;

	return exec(select);
}

/* Executa uma busca por soluções a partir de 'init' por 'iterTabu' vezes */
vector<Problema*>* Tabu::exec(Problema* init)
{
	multiset<Problema*, bool(*)(Problema*, Problema*)>* local;
	multiset<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;

	list<mov> *listaTabu = new list<mov>;

	Problema *maxGlobal = Problema::alloc(*init), *maxLocal = Problema::alloc(*init);

	// Loop principal
	for(int i = 0, j = 0; i < iterTabu && j < tentSemMelhora; i++, j++)
	{
		local = maxLocal->buscaLocal();

		// Pega a primeira solucao nao tabu
		for(iter = local->begin(); iter != local->end(); iter++)
		{
			// Se nao for tabu...
			if(!isTabu(listaTabu, (*iter)->movTabu))
			{
				if((*iter)->makespan < maxLocal->makespan)
					j = 0;

				delete maxLocal;
				maxLocal = Problema::alloc(**iter);
				if((*iter)->makespan < maxGlobal->makespan)
				{
					delete maxGlobal;
					maxGlobal = Problema::alloc(*maxLocal);
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
					j = 0;

					delete maxLocal;
					maxLocal = Problema::alloc(**iter);

					delete maxGlobal;
					maxGlobal = Problema::alloc(*maxLocal);

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

	return new vector<Problema*>(1, maxGlobal);
}

/* Verdadeiro se movimento avaliado for Tabu */
bool isTabu(list<mov> *listaTabu, mov m)
{
	list<mov>::iterator iter;

	for(iter = listaTabu->begin(); iter != listaTabu->end(); iter++)
		if(Problema::movTabuCMP(*iter, m))
			return true;

	return false;
}
