#include "Tabu.h"

using namespace std;

Tabu::Tabu() : Heuristica::Heuristica("DEFAULT_BT")
{
	numExec = 0;

	prob = 35;
	funcAsp = 0.5;
	polEscolha = 100;
	iterTabu = 750;
	tamListaTabu = 10;
	tentSemMelhora = 500;
	polExploracao = 0.5;

	Heuristica::numHeuristic += prob;
}

Tabu::Tabu(string nome, ParametrosHeuristicas& pBT) : Heuristica::Heuristica(nome)
{
	numExec = 0;

	prob = pBT.probBT != -1 ? pBT.probBT : 35;
	funcAsp = pBT.funcAspiracaoBT != -1 ? pBT.funcAspiracaoBT : 0.5;
	polEscolha = pBT.polEscolhaBT != -1 ? pBT.polEscolhaBT : 100;
	iterTabu = pBT.iterBT != -1 ? pBT.iterBT : 500;
	tamListaTabu = pBT.tamListaBT != -1 ? pBT.tamListaBT : 10;
	tentSemMelhora = pBT.tentSemMelhoraBT != -1 ? pBT.tentSemMelhoraBT : 250;
	polExploracao = pBT.polExplorBT != -1 ? pBT.polExplorBT : 0.5;

	Heuristica::numHeuristic += prob;
}

Tabu::~Tabu()
{
	Heuristica::numHeuristic -= prob;
}

/* Executa uma Busca Tabu na populacao com o devido criterio de selecao */
vector<Problema*>* Tabu::start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::const_iterator select;
	Problema* solBT;

	numExec++;

	pthread_mutex_lock(&mutex);

	if(polEscolha == 0)
	{
		select = sol->begin();
		solBT = Problema::copySoluction(**select);

		return exec(solBT);
	}

	// Escolhe alguem dentre os 'polEscolha' primeiras solucoes
	double visao = polEscolha < 0 ? Controle::sumFitnessMaximize(sol, sol->size()) : Controle::sumFitnessMaximize(sol, polEscolha);

	srand(randomic);

	// Evita trabalhar sobre solucoes ja selecionadas anteriormente
	select = Controle::selectRouletteWheel(sol, visao, rand());
	if(polEscolha < -1)
		while((*select)->exec.tabu == true)
			if(select != sol->begin())
				select--;
			else
				break;

	(*select)->exec.tabu = true;

	solBT = Problema::copySoluction(**select);

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
	vector<Problema*>* maxGlobal = new vector<Problema*>();
	Problema *maxLocal = init;

	maxGlobal->push_back(Problema::copySoluction(*maxLocal));

	// Loop principal
	for(int i = 0, j = 0; i < iterTabu && j < tentSemMelhora; i++, j++)
	{
		if(PARAR == true)
			break;

		if(polExploracao >= 1)
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
				if(*local->first < *maxLocal)
					j = 0;

				delete maxLocal;
				maxLocal = local->first;

				if(*local->first < *maxGlobal->back())
				{
					maxGlobal->push_back(Problema::copySoluction(*maxLocal));
				}

				addTabu(listaTabu, local->second, tamListaTabu);

				delete local;

				break;
			}
			// Eh tabu...
			else
			{
				// Satisfaz a funcao de aspiracao
				if(aspiracao((double)funcAsp, local->first, maxLocal, maxGlobal->back()))
				{
					j = 0;

					delete maxLocal;
					maxLocal = local->first;

					if(*local->first < *maxGlobal->back())
					{
						maxGlobal->push_back(Problema::copySoluction(*maxLocal));
					}

					delete local->second;
					delete local;

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

	return maxGlobal;
}

/* Verdadeiro se movimento avaliado for Tabu */
inline bool isTabu(list<movTabu*> *listaTabu, movTabu *m)
{
	list<movTabu*>::iterator iter;

	for(iter = listaTabu->begin(); iter != listaTabu->end(); iter++)
		if(*m == **iter)
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

inline bool aspiracao(double paramAsp, Problema *atual, Problema *local, Problema *global)
{
	return atual->getFitnessMinimize() < ((paramAsp * global->getFitnessMinimize()) + ((1-paramAsp) * local->getFitnessMinimize()));
}
