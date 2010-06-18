#include "Annealing.h"

using namespace std;

Annealing::Annealing() : Heuristica::Heuristica("DEFAULT_SA")
{
	numExec = 0;

	prob = 45;
	polEscolha = 10;
	maxIter = 250;
	initTemp = 125;
	fimTemp = 0.75;
	restauraSol = false;
	alfa = 0.99;

	Heuristica::numHeuristic += prob;
}

Annealing::Annealing(string nome, ParametrosHeuristicas& pSA) : Heuristica::Heuristica(nome)
{
	numExec = 0;

	prob = pSA.probSA != -1 ? pSA.probSA : 45;
	polEscolha = pSA.polEscolhaSA != -1 ? pSA.polEscolhaSA : 10;
	maxIter = pSA.maxIterSA != -1 ? pSA.maxIterSA : 250;
	initTemp = pSA.initTempSA != -1 ? pSA.initTempSA : 125;
	fimTemp = pSA.finalTempSA > 0 ? pSA.finalTempSA : 0.75;
	restauraSol = pSA.restauraSolSA != 0 ? true : false;
	alfa = pSA.alphaSA != -1 ? pSA.alphaSA : 0.99;

	Heuristica::numHeuristic += prob;
}

Annealing::~Annealing()
{
	Heuristica::numHeuristic -= prob;
}

/* Executa um Simulated Annealing na populacao com o devido criterio de selecao */
vector<Problema*>* Annealing::start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::const_iterator select;
	Problema* solSA;

	numExec++;

	pthread_mutex_lock(&mutex);

	if(polEscolha == 0)
	{
		select = sol->begin();
		solSA = Problema::alloc(**select);

		return exec(solSA);
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

	(*select)->exec.annealing = true;

	solSA = Problema::alloc(**select);

	pthread_mutex_unlock(&mutex);

	return exec(solSA);
}

/* Executa uma busca por solucoes a partir de 'init' */
vector<Problema*>* Annealing::exec(Problema* Si)
{
	vector<Problema*>* Sf = new vector<Problema*>();
	Problema *S, *Sn;
	double Ti, Tf, T;
	int L = maxIter;
	double Ds;

	Ti = initTemp != -1 ? initTemp : (Problema::best - Problema::worst) / log(0.5);
	Tf = fimTemp > 0 ? fimTemp : 1;

	T = Ti;
	S = Si;

	Sf->push_back(Problema::alloc(*Si));

	while(T > Tf)
	{
		if(PARAR == true)
			break;

		for(int i = 1; i < L; i++)
		{
			Sn = S->vizinho();

			if(Sn->getFitnessMinimize() == -1)
			{
				i--;
				delete Sn;
				continue;
			}

			Ds = Sn->getFitnessMinimize() - S->getFitnessMinimize();
			if(Ds <= 0 || accept((double)rand(), Ds, T))
			{
				delete S;
				S = Sn;

				if(*S < *Sf->back())
				{
					Sf->push_back(Problema::alloc(*S));
				}
			}
			else
			{
				delete Sn;
			}
		}
		T = alfa*T;

		if(restauraSol)
		{
			delete S;
			S = Problema::alloc(*Sf->back());
		}
	}
	delete S;

	return Sf;
}

inline bool accept(double rand, double Ds, double T)
{
	return ((rand/((float)RAND_MAX)) < (exp(-Ds/T)));
}
