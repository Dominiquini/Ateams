#include "JobShop.h"
#include "Heuristica.h"

using namespace std;

#ifndef _Tabu_
#define _Tabu_

class Tabu : public Heuristica
{
public:

	int iterTabu, tamListaTabu, tentSemMelhora;

	Tabu(ParametrosBT*);

	vector<Problema*>* start(set<Problema*, bool(*)(Problema*, Problema*)>* sol);

	vector<Problema*>* exec(Problema*);
};

bool isTabu(list<mov> *listaTabu, mov m);

bool mvcomp(mov, mov);

#endif