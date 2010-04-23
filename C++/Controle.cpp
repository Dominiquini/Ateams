#include "Controle.h"

using namespace std;

Controle::Controle(int maxPop, int iter, Tabu* classTabu)
{
	tamPop = maxPop;
	numAteams = iter;

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

	int i = 0;
	multiset<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	for(iter = pop->begin(); i < numAteams && iter != pop->end(); i++, iter++)
	{
		cout << "BT : " << i << " : " << (*pop->begin())->makespan << endl;
		pop->insert(algTabu->start(*iter));
	}

	return *(pop->begin());
}

void Controle::geraPop()
{
	srand(unsigned(time(NULL)));

	Problema* prob = NULL;
	while((int)pop->size() <= tamPop)
	{
		prob = new JobShop();

		if(prob->makespan != -1)
			pop->insert(prob);
		else
			delete prob;
	}
}
