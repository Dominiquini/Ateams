#include "Problema.h"
#include "JobShop.h"
#include "Tabu.h"
#include "Controle.h"

using namespace std;

extern bool PARAR;

Controle::Controle()
{
	tamPop = 500;
	numAteams = 50;
	maxTempo = INT_MAX;
	makespanBest = -1;

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = fncomp1;
	pop = new set<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	algs = new vector<Heuristica*>;
}

Controle::Controle(ParametrosATEAMS* pATEAMS)
{
	tamPop = pATEAMS->tamanhoPopulacao;
	numAteams = pATEAMS->iteracoesAteams;
	maxTempo = pATEAMS->maxTempo;
	makespanBest = pATEAMS->makespanBest;

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = fncomp1;
	pop = new set<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	algs = new vector<Heuristica*>;
}

Controle::~Controle()
{
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;

	for(iter = pop->begin(); iter != pop->end(); iter++)
		delete *iter;

	pop->clear();
	delete pop;

	vector<Heuristica*>::iterator it;

	for(it = algs->begin(); it != algs->end(); it++)
		Heuristica::numHeuristic -= (*it)->prob;

	algs->clear();
	delete algs;
}

void Controle::addHeuristic(Heuristica* alg)
{
	algs->push_back(alg);

	sort(algs->begin(), algs->end(), cmpAlg);
}

list<Problema*>* Controle::getPop()
{
	list<Problema*>* sol = new list<Problema*>();
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;

	for(iter = pop->begin(); iter != pop->end(); iter++)
		sol->push_back(*iter);

	return sol;
}

Problema* Controle::getSol(int n)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter = pop->begin();

	for(int i = 0; i <= n && iter != pop->end(); iter++);

	return *(--iter);
}

Problema* Controle::start()
{
	srand(unsigned(time(NULL)));

	geraPop();
	Problema::best = (*pop->rbegin())->getMakespan();

	cout << "CTR : 0 : " << (*pop->begin())->getMakespan() << endl << endl << flush;

	struct timeval time1, time2;
	gettimeofday(&time1, NULL);

	int tempo = 0, k;

	vector<Problema*>* prob;
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	pair<set<Problema*, bool(*)(Problema*, Problema*)>::iterator, bool> ret;
	for(int i = 0; i < numAteams && tempo < maxTempo; i++)
	{
		k = 0;
		prob = exec(rand());
		for(register int j = 0; j < (int)prob->size(); j++)
		{
			ret = pop->insert(prob->at(j));
			if(ret.second == true)
			{
				Problema::totalMakespan += prob->at(j)->getFitness();

				iter = pop->end();
				iter--;

				Problema::totalMakespan -= (*iter)->getFitness();
				pop->erase(iter);
				delete *iter;

				if(ret.first != iter)
					k++;
			}
			else
			{
				delete prob->at(j);
			}
		}
		prob->clear();
		delete prob;

		cout << atual << " : " << i+1 << " : " << (*pop->begin())->getMakespan() << " -> " << k << endl << flush;

		if((*pop->begin())->getMakespan() <= makespanBest)
			break;

		if(PARAR == 1)
			break;

		gettimeofday(&time2, NULL);
		tempo = time2.tv_sec - time1.tv_sec;
	}
	return *(pop->begin());
}

vector<Problema*>* Controle::exec(int randomic)
{
	Heuristica* alg = selectRouletteWheel(algs, Heuristica::numHeuristic, randomic);
	atual = alg->name;
	return alg->start(pop, randomic);
}

void Controle::geraPop()
{
	srand(unsigned(time(NULL)));

	Problema* prob = NULL;
	pair<set<Problema*, bool(*)(Problema*, Problema*)>::iterator, bool> ret;
	while((int)pop->size() < tamPop)
	{
		prob = Problema::alloc();

		if(prob->getMakespan() != -1)
		{
			ret = pop->insert(prob);
			if(ret.second == true)
				Problema::totalMakespan += prob->getFitness();
			else
				delete prob;
		}
		else
			delete prob;
	}
}

set<Problema*, bool(*)(Problema*, Problema*)>::iterator Controle::selectRouletteWheel(set<Problema*, bool(*)(Problema*, Problema*)>* pop, int fitTotal, int randWheel)
{
	// Armazena o fitness total da populacao
	int sum = fitTotal;
	// Um numero entre zero e "sum" e sorteado
	randWheel = randWheel % (sum + 1);

	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	for(iter = pop->begin(); iter != pop->end(); iter++)
	{
		sum -= (int)(*iter)->getFitness();
		if(sum <= randWheel)
		{
			return iter;
		}
	}
	return (pop->begin());
}

vector<Problema*>::iterator Controle::selectRouletteWheel(vector<Problema*>* pop, int fitTotal, int randWheel)
{
	// Armazena o fitness total da populacao
	int sum = fitTotal;
	// Um numero entre zero e "sum" e sorteado
	randWheel = randWheel % (sum + 1);

	vector<Problema*>::iterator iter;
	for(iter = pop->begin(); iter != pop->end(); iter++)
	{
		sum -= (int)(*iter)->getFitness();
		if(sum <= randWheel)
		{
			return iter;
		}
	}
	return pop->begin();
}

Heuristica* Controle::selectRouletteWheel(vector<Heuristica*>* heuristc, int probTotal, int randWheel)
{
	// Armazena o fitness total da populacao
	int sum = probTotal;
	// Um numero entre zero e "sum" e sorteado
	randWheel = randWheel % (sum + 1);

	for(int i = 0; i < (int)heuristc->size(); i++)
	{
		sum -= heuristc->at(i)->prob;
		if(sum <= randWheel)
		{
			return heuristc->at(i);
		}
	}
	return heuristc->at(0);
}

inline bool cmpAlg(Heuristica *h1, Heuristica *h2)
{
	return h1->prob < h2->prob;
}

int Heuristica::numHeuristic = 0;
