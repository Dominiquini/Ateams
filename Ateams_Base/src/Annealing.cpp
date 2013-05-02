#include "Annealing.hpp"

using namespace std;

Annealing::Annealing() : Heuristica::Heuristica("DEFAULT_SA")
{
	numExec = 0;

	prob = 52;
	polEscolha = 10;
	maxIter = 250;
	initTemp = 125;
	fimTemp = 0.75;
	restauraSol = false;
	alfa = 0.99;
	elitismo = 20;

	Heuristica::numHeuristic += prob;
}

Annealing::~Annealing()
{
	Heuristica::numHeuristic -= prob;
}


bool Annealing::setParameter(const char* parameter, const char* value)
{
	if(Heuristica::setParameter(parameter, value))
		return true;

	if(strcasecmp(parameter, "probSA") == 0)
	{
		Heuristica::numHeuristic -= prob;
		sscanf(value, "%d", &prob);
		Heuristica::numHeuristic += prob;
	}
	else if(strcasecmp(parameter, "polEscolhaSA") == 0)
	{
		sscanf(value, "%d", &polEscolha);
	}
	else if(strcasecmp(parameter, "probElitismoSA") == 0)
	{
		sscanf(value, "%d", &elitismo);
	}
	else if(strcasecmp(parameter, "maxIterSA") == 0)
	{
		sscanf(value, "%d", &maxIter);
	}
	else if(strcasecmp(parameter, "initTempSA") == 0)
	{
		sscanf(value, "%f", &initTemp);
	}
	else if(strcasecmp(parameter, "finalTempSA") == 0)
	{
		sscanf(value, "%f", &fimTemp);
	}
	else if(strcasecmp(parameter, "restauraSolSA") == 0)
	{
		int temp;

		sscanf(value, "%d", &temp);

		if(temp == 1)
			restauraSol = true;
		else
			restauraSol = false;
	}
	else if(strcasecmp(parameter, "alphaSA") == 0)
	{
		sscanf(value, "%f", &alfa);
	}
	else
	{
		return false;
	}

	return true;
}

/* Executa um Simulated Annealing na populacao com o devido criterio de selecao */
vector<Problema*>* Annealing::start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic, Heuristica_Listener* listener)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::const_iterator select;
	Problema* solSA;

	srand(randomic);

	numExec++;

	pthread_mutex_lock(&mutex_pop);

	// Escolhe a melhor solucao para ser usada pelo SA
	if(polEscolha == 0 || xRand(rand(), 0, 101) < elitismo)
	{
		select = sol->begin();
		solSA = Problema::copySoluction(**select);

		pthread_mutex_unlock(&mutex_pop);

		return exec(solSA, listener);
	}

	// Escolhe alguem dentre os 'polEscolha' primeiras solucoes
	double visao = polEscolha < 0 ? Controle::sumFitnessMaximize(sol, sol->size()) : Controle::sumFitnessMaximize(sol, polEscolha);

	srand(randomic);

	// Evita trabalhar sobre solucoes ja selecionadas anteriormente
	select = Controle::selectRouletteWheel(sol, visao, rand());
	if(polEscolha < -1)
		while((*select)->exec.annealing == true)
			if(select != sol->begin())
				select--;
			else
				break;

	(*select)->exec.annealing = true;

	solSA = Problema::copySoluction(**select);

	pthread_mutex_unlock(&mutex_pop);

	return exec(solSA, listener);
}

/* Executa uma busca por solucoes a partir de 'init' */
vector<Problema*>* Annealing::exec(Problema* Si, Heuristica_Listener* listener)
{
	vector<Problema*>* Sf = new vector<Problema*>();
	Problema *S, *Sn;
	double Ti, Tf, T;
	int L = maxIter;
	double Ds;

	Ti = initTemp != -1 ? initTemp : (Problema::best - Problema::worst) / log(0.5);
	Tf = fimTemp > 0 ? fimTemp : 1;

	double diff = Ti - Tf;

	T = Ti;
	S = Si;

	Sf->push_back(Problema::copySoluction(*Si));

	if(listener != NULL)
		listener->bestInitialFitness = (*Sf->begin())->getFitness();

	bool exec = true;
	while(exec)
	{
		if(PARAR == true)
			break;

		if(listener != NULL)
		{
			listener->status = 100.00 - (100.00 * (T - Tf)) / diff;

			listener->bestActualFitness = (*Sf->rbegin())->getFitness();

			listener->setInfo("Temperatura: %f", T);
		}

		if(T == Tf)
			exec = false;

		for(int i = 1; i < L; i++)
		{
			Sn = S->vizinho();

			if(Sn == NULL)
			{
				i--;
				continue;
			}

			Ds = Problema::melhora(*S, *Sn);
			if(Ds >= 0 || accept((double)rand(), Ds, T))
			{
				delete S;
				S = Sn;

				if(Problema::melhora(*Sf->back(), *S) > 0)
				{
					Sf->push_back(Problema::copySoluction(*S));
				}
			}
			else
			{
				delete Sn;
			}
		}
		T = alfa*T;

		if(T < Tf)
			T = Tf;

		if(restauraSol)
		{
			delete S;
			S = Problema::copySoluction(*Sf->back());
		}
	}
	delete S;

	return Sf;
}

inline bool accept(double rand, double Ds, double T)
{
	return ((rand/((float)RAND_MAX)) < (exp(Ds/T)));
}
