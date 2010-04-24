#include "Problema.h"
#include "JobShop.h"
#include "Tabu.h"
#include "Controle.h"

using namespace std;

extern int PARAR;

Controle::Controle(ParametrosATEAMS* pATEAMS, Tabu* classTabu)
{
	tamPop = pATEAMS->tamanhoPopulacao;
	numAteams = pATEAMS->iteracoesAteams;
	maxTempo = pATEAMS->maxTempo;

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = fncomp;
	pop = new multiset<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	algTabu = classTabu;
}

Controle::~Controle()
{
	multiset<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;

	for(iter = pop->begin(); iter != pop->end(); iter++)
		delete *iter;

	pop->clear();
	delete pop;
}

Problema* Controle::start()
{
	geraPop();

	struct timeval time1, time2;
	gettimeofday(&time1, NULL);

	int tempo = 0;

	Problema* prob;
	for(int i = 0; i < numAteams && tempo < maxTempo; i++)
	{
		prob = algTabu->start(pop);
		if(prob->movTabu.maq != -1)
		{
			multiset<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
			iter = pop->end();
			iter--;

			pop->insert(prob);
			Problema::totalMakespan += prob->getFitness();

			Problema::totalMakespan -= (*iter)->getFitness();
			pop->erase(iter);
			delete *iter;
		}

		cout.precision(10);
		cout << "BT : " << i << " : " << (*pop->begin())->makespan << endl << flush;

		if(PARAR == 1)
			break;

		gettimeofday(&time2, NULL);
		tempo = time2.tv_sec - time1.tv_sec;
	}
	return *(pop->begin());
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
