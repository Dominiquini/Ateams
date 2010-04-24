#include "JobShop.h"

using namespace std;

#ifndef _Tabu_
#define _Tabu_

class Tabu
{
public:

	int iterTabu, tamListaTabu, tentSemMelhora;

	Tabu(ParametrosBT*);

	multiset<Problema*, bool(*)(Problema*, Problema*)>* start(multiset<Problema*, bool(*)(Problema*, Problema*)>* sol);

	multiset<Problema*, bool(*)(Problema*, Problema*)>* exec(Problema*);
};

bool isTabu(list<mov> *listaTabu, mov m);

bool mvcomp(mov, mov);

#endif
