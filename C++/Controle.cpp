#include "Controle.h"

using namespace std;

Controle::Controle(int maxPop, char* problema)
{
	tamPop = maxPop;
	Problema::leProblema(fopen(problema, "r"));
}

Problema* Controle::start()
{
	set<Problema*, bool(*)(Problema*, Problema*)>* sol = geraPop();

	return *(sol->begin());
}

set<Problema*, bool(*)(Problema*, Problema*)>* Controle::geraPop()
{
	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = fncomp;
	set<Problema*, bool(*)(Problema*, Problema*)>* sol;

	sol = new set<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	while(sol->size() <= tamPop)
	{
		Problema* prob = new JobShop();
		if(prob->makespan != -1)
			sol->insert(prob);
		else
			delete prob;
	}
	return sol;
}
