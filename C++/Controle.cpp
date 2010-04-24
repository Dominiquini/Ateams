#include "Controle.h"

using namespace std;

extern int PARAR;

Controle::Controle(ParametrosATEAMS* pATEAMS, Tabu* classTabu)
{
	tamPop = pATEAMS->tamanhoPopulacao;
	numAteams = pATEAMS->iteracoesAteams;

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

		if(PARAR == 1)
			break;
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
