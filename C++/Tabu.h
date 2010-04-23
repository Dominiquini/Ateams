#include "JobShop.h"

using namespace std;

#ifndef _Tabu_
#define _Tabu_

class Tabu
{
public:

	int iterTabu, tamListaTabu;

	Tabu(int iter, int lista);

	Problema* start(Problema*);
};

bool isTabu(list<mov> *listaTabu, mov m);

bool mvcomp(mov, mov);

#endif
