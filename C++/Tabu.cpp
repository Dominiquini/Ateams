#include "Tabu.h"

using namespace std;

extern bool PARAR;

Tabu::Tabu()
{
	numExec = 0;

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
	numExec = 0;

	name = "BT";
	prob = pBT->probBT;
	funcAsp = pBT->funcAsp;
	polEscolha = pBT->polEscolha;
	iterTabu = pBT->numeroIteracoes;
	tamListaTabu = pBT->tamanhoListaTabu;
	tentSemMelhora = pBT->tentativasSemMelhora;

	Heuristica::numHeuristic += prob;
}

Tabu::~Tabu()
{
	Heuristica::numHeuristic -= prob;
}

/* Executa uma Busca Tabu na populacao com o devido criterio de selecao */
vector<Problema*>* Tabu::start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator select;
	Problema* solBT;

	numExec++;

	if(polEscolha == 0)
	{
		pthread_mutex_lock(&mutex);
		select = sol->begin();
		solBT = Problema::alloc(**select);
		pthread_mutex_unlock(&mutex);

		return exec(solBT);
	}

	// Escolhe alguem dentre os 'polEscolha' primeiras solucoes
	double visao = polEscolha < 0 ? Problema::totalMakespan : Problema::sumFitness(sol, polEscolha);

	// Evita trabalhar sobre solucoes ja selecionadas anteriormente
	pthread_mutex_lock(&mutex);
	select = Controle::selectRouletteWheel(sol, (int)visao, randomic);
	if(polEscolha == -1)
		while((*select)->exec.tabu == true)
			if(select != sol->begin())
				select--;
			else
				break;

	(*select)->exec.tabu = true;

	solBT = Problema::alloc(**select);
	pthread_mutex_unlock(&mutex);

	return exec(solBT);
}

/* Executa uma busca por solucoes a partir de 'init' por 'iterTabu' vezes */
vector<Problema*>* Tabu::exec(Problema* init)
{
	vector<pair<Problema*, tTabu*>* >* local;
	vector<pair<Problema*, tTabu*>* >::iterator iter;

	// Lista Tabu de movimentos repetidos
	list<tTabu> *listaTabu = new list<tTabu>;

	// Maximos globais e locais na execucao da Busca Tabu
	Problema *maxGlobal = Problema::alloc(*init), *maxLocal = init;

	// Loop principal
	for(int i = 0, j = 0; i < iterTabu && j < tentSemMelhora; i++, j++)
	{
		if(PARAR == true)
			break;

		// Pega uma lista de todos os "vizinhos" de maxLocal
		local = maxLocal->buscaLocal();

		// Pega a primeira solucao nao tabu
		for(iter = local->begin(); iter != local->end(); iter++)
		{
			// Se nao for tabu...
			if(!isTabu(listaTabu, (*iter)->second))
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
				addTabu(listaTabu, (*iter)->second, tamListaTabu);

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
inline bool isTabu(list<tTabu> *listaTabu, tTabu *m)
{
	list<tTabu>::iterator iter;

	for(iter = listaTabu->begin(); iter != listaTabu->end(); iter++)
		if(Problema::movTabuCMP(*iter, *m))
			return true;

	return false;
}

inline void addTabu(list<tTabu>* listaTabu, tTabu *m, int max)
{
	listaTabu->push_front(*m);
	if((int)listaTabu->size() > max)
		listaTabu->pop_back();
}
