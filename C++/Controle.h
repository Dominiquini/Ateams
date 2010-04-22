#include "JobShop.h"

using namespace std;

#ifndef _Controle_
#define _Controle_

class Controle
{
public:
	int tamPop;

	Controle(int maxPop, char* problema);

	Problema* start();

	set<Problema*, bool(*)(Problema*, Problema*)>* geraPop();
};

#endif
