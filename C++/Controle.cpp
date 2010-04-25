#include "Problema.h"
#include "JobShop.h"
#include "Tabu.h"
#include "Controle.h"

using namespace std;

extern int PARAR;

Controle::Controle(ParametrosATEAMS* pATEAMS)
{
	tamPop = pATEAMS->tamanhoPopulacao;
	numAteams = pATEAMS->iteracoesAteams;
	maxTempo = pATEAMS->maxTempo;
	makespanBest = pATEAMS->makespanBest;

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = fncomp;
	pop = new multiset<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	algs = new vector<Heuristica*>;
}

Controle::~Controle()
{
	multiset<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;

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
	geraPop();

	cout << "CTR : 0 : " << (*pop->begin())->makespan << endl << flush;

	struct timeval time1, time2;
	gettimeofday(&time1, NULL);

	int tempo = 0;

	vector<Problema*>* prob;
	multiset<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	for(int i = 0; i < numAteams && tempo < maxTempo; i++)
	{
		prob = exec();
		for(int j = 0; j < (int)prob->size(); j++)
		{
			pop->insert(prob->at(j));
			Problema::totalMakespan += prob->at(j)->getFitness();

			iter = pop->end();
			iter--;

			Problema::totalMakespan -= (*iter)->getFitness();
			pop->erase(iter);
			delete *iter;
		}
		prob->clear();
		delete prob;

		cout << atual << " : " << i+1 << " : " << (*pop->begin())->makespan << endl << flush;

		if((*pop->begin())->makespan <= makespanBest)
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

vector<Problema*>* Controle::exec()
{
	Heuristica* alg = selectRouletteWheel(algs, Heuristica::numHeuristic);
	atual = alg->name;
	return alg->start(pop);
}

void Controle::geraPop()
{
	srand(unsigned(time(NULL)));

	Problema* prob = NULL;
	while((int)pop->size() <= tamPop)
	{
		prob = Problema::alloc();

		if(prob->makespan != -1)
		{
			pop->insert(prob);
			Problema::totalMakespan += prob->getFitness();
		}
		else
			delete prob;
	}
}

Problema* Controle::selectRouletteWheel(multiset<Problema*, bool(*)(Problema*, Problema*)>* pop, int fitTotal)
{
	// Armazena o fitness total da população
	int sum = fitTotal;
	// Um número entre zero e "sum" é sorteado
	srand(unsigned(time(NULL)));
	int randWheel = rand() % (sum + 1);

	if(rand()%101 <  25)
		sum = 0;

	multiset<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	for(iter = pop->begin(); iter != pop->end(); iter++)
	{
		sum -= (*iter)->getFitness();
		if(sum <= randWheel)
		{
			return *iter;
		}
	}
	return *(pop->begin());
}

Heuristica* Controle::selectRouletteWheel(vector<Heuristica*>* heuristc, int probTotal)
{
	// Armazena o fitness total da população
	int sum = probTotal;
	// Um número entre zero e "sum" é sorteado
	srand(unsigned(time(NULL)));
	int randWheel = rand() % (sum + 1);

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
