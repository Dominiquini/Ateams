#include "Tabu.h"

using namespace std;

extern bool PARAR;

Tabu::Tabu()
{
	numExec = 0;

	name = "DEFAULT_BT";
	prob = 35;
	funcAsp = 0.5;
	polEscolha = 100;
	iterTabu = 500;
	tamListaTabu = 10;
	tentSemMelhora = 250;
	polExploracao = 0.25;

	Heuristica::numHeuristic += prob;
}

Tabu::Tabu(ParametrosBT* pBT)
{
	numExec = 0;

	name = "BT";
	prob = pBT->probBT != -1 ? pBT->probBT : 35;
	funcAsp = pBT->funcAsp != -1 ? pBT->funcAsp : 0.5;
	polEscolha = pBT->polEscolha != -1 ? pBT->polEscolha : 100;
	iterTabu = pBT->numeroIteracoes != -1 ? pBT->numeroIteracoes : 500;
	tamListaTabu = pBT->tamanhoListaTabu != -1 ? pBT->tamanhoListaTabu : 10;
	tentSemMelhora = pBT->tentativasSemMelhora != -1 ? pBT->tentativasSemMelhora : 250;
	polExploracao = pBT->polExploracao != -1 ? pBT->polExploracao : 0.25;

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
	double visao = polEscolha < 0 ? Problema::sumFitnessMaximize(sol, sol->size()) : Problema::sumFitnessMaximize(sol, polEscolha);

	srand(randomic);

	// Evita trabalhar sobre solucoes ja selecionadas anteriormente
	pthread_mutex_lock(&mutex);
	select = Controle::selectRouletteWheel(sol, visao, rand());
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
	vector<pair<Problema*, movTabu*>* >* vizinhanca;
	pair<Problema*, movTabu*>* local;

	// Lista Tabu de movimentos repetidos
	list<movTabu*>* listaTabu = new list<movTabu*>;

	// Maximos globais e locais na execucao da Busca Tabu
	Problema *maxGlobal = Problema::alloc(*init), *maxLocal = init;

	// Loop principal
	for(int i = 0, j = 0; i < iterTabu && j < tentSemMelhora; i++, j++)
	{
		if(PARAR == true)
			break;

		if(polExploracao == -1)
		{
			// Pega uma lista de todos os "vizinhos" de maxLocal
			vizinhanca = maxLocal->buscaLocal();
		}
		else
		{
			// Pega uma parcela 'polExploracao' dos "vizinhos" de maxLocal
			vizinhanca = maxLocal->buscaLocal(polExploracao);
		}

		// Escolhe a solucao de peso minimo
		while(!vizinhanca->empty())
		{
			local = vizinhanca->back();
			vizinhanca->pop_back();

			// Se nao for tabu...
			if(!isTabu(listaTabu, local->second))
			{
				if(local->first->getFitnessMinimize() < maxLocal->getFitnessMinimize())
					j = 0;

				delete maxLocal;
				maxLocal = local->first;

				if(local->first->getFitnessMinimize() < maxGlobal->getFitnessMinimize())
				{
					delete maxGlobal;
					maxGlobal = Problema::alloc(*maxLocal);
				}

				addTabu(listaTabu, local->second, tamListaTabu);

				delete local;

				break;
			}
			// Eh tabu...
			else
			{
				// Satisfaz a funcao de aspiracao
				if(local->first->getFitnessMinimize() < aspiracao((double)funcAsp, maxLocal->getFitnessMinimize(), maxGlobal->getFitnessMinimize()))
				{
					j = 0;

					delete maxLocal;
					maxLocal = local->first;

					if(local->first->getFitnessMinimize() < maxGlobal->getFitnessMinimize())
					{
						delete maxGlobal;
						maxGlobal = Problema::alloc(*maxLocal);
					}

					delete local;
					delete local->second;

					break;
				}
				else
				{
					delete local->first;
					delete local->second;
					delete local;
				}
			}
		}
		while(!vizinhanca->empty())
		{
			local = vizinhanca->back();
			vizinhanca->pop_back();

			delete local->first;
			delete local->second;
			delete local;
		}
		vizinhanca->clear();
		delete vizinhanca;
	}
	delete maxLocal;

	while(!listaTabu->empty())
	{
		delete listaTabu->back();
		listaTabu->pop_back();
	}
	listaTabu->clear();
	delete listaTabu;

	maxGlobal->exec.tabu = true;

	return new vector<Problema*>(1, maxGlobal);
}

/* Verdadeiro se movimento avaliado for Tabu */
inline bool isTabu(list<movTabu*> *listaTabu, movTabu *m)
{
	list<movTabu*>::iterator iter;

	for(iter = listaTabu->begin(); iter != listaTabu->end(); iter++)
		if(m->movTabuCMP(**iter))
			return true;

	return false;
}

inline void addTabu(list<movTabu*>* listaTabu, movTabu *m, int max)
{
	listaTabu->push_front(m);

	if((int)listaTabu->size() > max)
	{
		delete listaTabu->back();
		listaTabu->pop_back();
	}
}

inline double aspiracao(double paramAsp, double local, double global)
{
	return ((paramAsp*global) + ((1-paramAsp)*local));
}
