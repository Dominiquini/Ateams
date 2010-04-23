#include "JobShop.h"
#include "Tabu.h"

using namespace std;

#ifndef _Controle_
#define _Controle_

class Controle
{
public:
	Tabu* algTabu;

	int tamPop, numAteams;
	multiset<Problema*, bool(*)(Problema*, Problema*)>* pop;

	Controle(int maxPop, int iter, Tabu* classTabu);

	~Controle();

	Problema* start();

	void geraPop();
};

#endif
