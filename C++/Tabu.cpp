#include "Controle.h"
#include "Tabu.h"

using namespace std;

extern bool PARAR;

Tabu::Tabu()
{
	name = "DEFAULT_BT";
	prob = 60;
	funcAsp = 1;
	polEscolha = -1;
	iterTabu = 500;
	tamListaTabu = 25;
	tentSemMelhora = 50;

	Heuristica::numHeuristic += prob;
}

Tabu::Tabu(ParametrosBT* pBT)
{
	name = "BT";
	prob = pBT->probBT;
	funcAsp = pBT->funcAsp;
	polEscolha = pBT->polEscolha;
	iterTabu = pBT->numeroIteracoes;
	tamListaTabu = pBT->tamanhoListaTabu;
	tentSemMelhora = pBT->tentativasSemMelhora;

	Heuristica::numHeuristic += prob;
}

/* Executa uma Busca Tabu na populacao com o devido criterio de selecao */
vector<Problema*>* Tabu::start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator select = sol->begin();

	if(polEscolha == 0)
		return exec(*select);

	// Escolhe alguem dentre os 'pollEscolha' primeiras solucoes
	double visao = polEscolha < 0 ? Problema::totalMakespan : Problema::sumFitness(sol, polEscolha);

	// Evita trabalhar sobre solucoes ja selecionadas anteriormente
	select = Controle::selectRouletteWheel(sol, visao, randomic);
	if(polEscolha == -1)
		while((*select)->exec.tabu == true)
			if(select != sol->begin())
				select--;
			else
				break;

	(*select)->exec.tabu = true;

	return exec(*select);
}

/* Executa uma busca por solucoes a partir de 'init' por 'iterTabu' vezes */
vector<Problema*>* Tabu::exec(Problema* init)
{
	vector<pair<Problema*, tTabu*>* >* local;
	vector<pair<Problema*, tTabu*>* >::iterator iter;

	// Lista Tabu de movimentos repetidos
	list<tTabu> *listaTabu = new list<tTabu>;

	// Maximos globais e locais na execucao da Busca Tabu
	Problema *maxGlobal = Problema::alloc(*init), *maxLocal = Problema::alloc(*init);

	// Loop principal
	for(int i = 0, j = 0; i < iterTabu && j < tentSemMelhora; i++, j++)
	{
		if(PARAR == 1)
			break;

		// Pega uma lista de todos os "vizinhos" de maxLocal
		local = maxLocal->buscaLocal();

		// Pega a primeira solucao nao tabu
		for(iter = local->begin(); iter != local->end(); iter++)
		{
			// Se nao for tabu...
			if(!isTabu(listaTabu, *(*iter)->second))
			{
				if((*iter)->first->getMakespan() < maxLocal->getMakespan())
					j = 0;

				delete maxLocal;
				maxLocal = Problema::alloc(*(*iter)->first);
				if((*iter)->first->getMakespan() < maxGlobal->getMakespan())
				{
					delete maxGlobal;
					maxGlobal = Problema::alloc(*maxLocal);
				}
				listaTabu->push_front(*(*iter)->second);
				if((int)listaTabu->size() > tamListaTabu)
					listaTabu->pop_back();

				break;
			}
			// Eh tabu...
			else
			{
				// Satisfaz a funcao de aspiracao
				if((*iter)->first->getMakespan() < ((funcAsp*maxGlobal->getMakespan()) + ((1-funcAsp)*maxLocal->getMakespan())))
				{
					j = 0;

					delete maxLocal;
					maxLocal = Problema::alloc(*(*iter)->first);

					delete maxGlobal;
					maxGlobal = Problema::alloc(*maxLocal);

					break;
				}
			}
		}

		/* Libera espaco da buscaLocal() */
		for(iter = local->begin(); iter != local->end(); iter++)
		{
			delete (*iter)->first;
			free((*iter)->second);
			delete *iter;
		}
		local->clear();
		delete local;
	}

	delete listaTabu;
	delete maxLocal;

	maxGlobal->exec.tabu = true;

	return new vector<Problema*>(1, maxGlobal);
}

/* Verdadeiro se movimento avaliado for Tabu */
inline bool isTabu(list<tTabu> *listaTabu, tTabu m)
{
	list<tTabu>::iterator iter;

	for(iter = listaTabu->begin(); iter != listaTabu->end(); iter++)
		if(Problema::movTabuCMP(*iter, m))
			return true;

	return false;
}
