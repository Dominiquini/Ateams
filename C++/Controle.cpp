#include "Problema.h"
#include "JobShop.h"
#include "Tabu.h"
#include "Controle.h"

using namespace std;

extern int PARAR;

Controle::Controle()
{
	tamPop = 500;
	numAteams = 50;
	maxTempo = INT_MAX;
	makespanBest = -1;

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = fncomp;
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

	bool(*fn_pt)(Problema*, Problema*) = fncomp;
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

	algs->clear();
	delete algs;
}

void Controle::addHeuristic(Heuristica* alg)
{
	algs->push_back(alg);
}

Problema* Controle::start()
{
	srand(unsigned(time(NULL)));

	geraPop();

	cout << "CTR : 0 : " << (*pop->begin())->getMakespan() << endl << flush;

	struct timeval time1, time2;
	gettimeofday(&time1, NULL);

	int tempo = 0;

	vector<Problema*>* prob;
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	pair<set<Problema*, bool(*)(Problema*, Problema*)>::iterator, bool> ret;
	for(int i = 0; i < numAteams && tempo < maxTempo; i++)
	{
		prob = exec(rand());
		for(int j = 0; j < (int)prob->size(); j++)
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
			}
			else
			{
				delete prob->at(j);
			}
		}
		prob->clear();
		delete prob;

		cout << atual << " : " << i+1 << " : " << (*pop->begin())->getMakespan() << endl << flush;

		if((*pop->begin())->getMakespan() <= makespanBest)
		{
			cout << endl << "Populacao ATEAMS Convergiu na " << i+1 << " iteracao" << endl;
			break;
		}

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
	return alg->start(pop);
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

set<Problema*, bool(*)(Problema*, Problema*)>::iterator Controle::selectRouletteWheel(set<Problema*, bool(*)(Problema*, Problema*)>* pop, int fitTotal)
{
	// Armazena o fitness total da populacao
	int sum = fitTotal;
	// Um numero entre zero e "sum" e sorteado
	srand(unsigned(time(NULL)));
	int randWheel = rand() % (sum + 1);

	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	for(iter = pop->begin(); iter != pop->end(); iter++)
	{
		sum -= (*iter)->getFitness();
		if(sum <= randWheel)
		{
			return iter;
		}
	}
	return (pop->begin());
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

int Heuristica::numHeuristic = 0;
