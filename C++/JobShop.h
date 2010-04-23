#include <stdlib.h>
#include <time.h>

#include "Problema.h"

using namespace std;

#ifndef _JobShop_
#define _JobShop_

class JobShop : public Problema
{
public:

	JobShop();
	JobShop(int **prob);
	JobShop(Problema &prob);
	JobShop(Problema &prob, int maq, int pos1, int pos2);

	int calcMakespan();
	void imprimir();

	multiset<Problema*, bool(*)(Problema*, Problema*)>* buscaLocal();
};


int findOrdem(int M, int maq, int* job);

void* alocaMatriz(int, int, int, int);

void desalocaMatriz(int, void*, int, int);

bool fncomp(Problema*, Problema*);

ptrdiff_t myrandom (ptrdiff_t i);

#endif
