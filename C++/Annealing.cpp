#include "Annealing.h"

using namespace std;

extern bool PARAR;

Annealing::Annealing()
{
	numExec = 0;

	name = "DEFAULT_SA";
	prob = 45;
	polEscolha = 100;
	maxIter = 125;
	initTemp = 100;
	fimTemp = 1;
	restauraSol = false;
	alfa = 0.99;

	Heuristica::numHeuristic += prob;
}

Annealing::Annealing(ParametrosSA *pSA)
{
	numExec = 0;

	name = "SA";
	prob = pSA->probSA != -1 ? pSA->probSA : 45;
	polEscolha = pSA->polEscolha != -1 ? pSA->polEscolha : 100;
	maxIter = pSA->maxIter != -1 ? pSA->maxIter : 125;
	initTemp = pSA->initTemp != -1 ? pSA->initTemp : 100;
	fimTemp = pSA->fimTemp > 0 ? pSA->fimTemp : 1;
	restauraSol = pSA->restauraSol != 0 ? true : false;
	alfa = pSA->alfa != -1 ? pSA->alfa : 0.99;

	Heuristica::numHeuristic += prob;
}

Annealing::~Annealing()
{
	Heuristica::numHeuristic -= prob;
}

/* Executa um Simulated Annealing na populacao com o devido criterio de selecao */
vector<Problema*>* Annealing::start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic)
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

	(*select)->exec.annealing = true;

	solBT = Problema::alloc(**select);
	pthread_mutex_unlock(&mutex);

	return exec(solBT);
}

/* Executa uma busca por solucoes a partir de 'init' */
vector<Problema*>* Annealing::exec(Problema* Si)
{
	Problema *Sf, *S, *Sn;
	double Ti, Tf, T;
	int L = maxIter;
	double Ds;

	Ti = initTemp != -1 ? initTemp : (Problema::best - Problema::worst) / log(0.5);
	Tf = fimTemp > 0 ? fimTemp : 1;

	T = Ti;
	S = Si;
	Sf = Problema::alloc(*Si);

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

				if(S->getFitnessMinimize() < Sf->getFitnessMinimize())
				{
					delete Sf;
					Sf = Problema::alloc(*S);
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
			S = Problema::alloc(*Sf);
		}
	}
	delete S;

	return new vector<Problema*>(1, Sf);
}

inline bool accept(double rand, double Ds, double T)
{
	return ((rand/((float)RAND_MAX)) < (exp(-Ds/T)));
}
