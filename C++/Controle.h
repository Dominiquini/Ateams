#include "JobShop.h"

using namespace std;

#ifndef _Controle_
#define _Controle_

class Controle
{
public:
	int tamPop;
	multiset<Problema*, bool(*)(Problema*, Problema*)>* pop;

	Controle(int maxPop, char* problema);

	~Controle();

	Problema* start();

	void geraPop();
};

#endif
