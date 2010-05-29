#include "Annealing.h"

using namespace std;

extern bool PARAR;

Annealing::Annealing()
{
	numExec = 0;

	name = "DEFAULT_SA";
	prob = 50;
	polEscolha = 10;
	maxIter = 100;
	initTemp = 100;
	fimTemp = 1;
	alfa = 0.99;

	Heuristica::numHeuristic += prob;
}

Annealing::Annealing(ParametrosSA *pAS)
{
	numExec = 0;

	name = "SA";
	prob = pAS->probSA;
	polEscolha = pAS->polEscolha;
	maxIter = pAS->maxIter;
	initTemp = pAS->initTemp;
	fimTemp = pAS->fimTemp;
	alfa = pAS->alfa;

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
	double visao = polEscolha < 0 ? Problema::totalMakespan : Problema::sumFitness(sol, polEscolha);

	// Evita trabalhar sobre solucoes ja selecionadas anteriormente
	pthread_mutex_lock(&mutex);
	select = Controle::selectRouletteWheel(sol, (int)visao, randomic);

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

	Ti = initTemp;
	Tf = fimTemp;

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

			if(Sn->getMakespan() == -1)
			{
				i--;
				delete Sn;
				continue;
			}

			Ds = Sn->getMakespan() - S->getMakespan();
			if(Ds < 0 || accept((double)rand(), Ds, T))
			{
				delete S;
				S = Sn;

				if(S->getMakespan() < Sf->getMakespan())
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
	}

	delete S;

	return new vector<Problema*>(1, Sf);
}

inline bool accept(double rand, double Ds, double T)
{
	double u = rand/((float)RAND_MAX);
	double a = exp(-Ds/T);

	return (u < a);
}
