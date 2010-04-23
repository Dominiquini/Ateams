#include "Controle.h"

using namespace std;

Controle::Controle(int maxPop, char* problema)
{
	tamPop = maxPop;
	Problema::leProblema(fopen(problema, "r"));

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = fncomp;
	pop = new multiset<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);
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

	return *(pop->begin());
}

void Controle::geraPop()
{
	srand(unsigned(time(NULL)));

	Problema* prob = NULL;
	while(pop->size() <= tamPop)
	{
		prob = new JobShop();

		if(prob->makespan != -1)
			pop->insert(prob);
		else
			delete prob;
	}
}
