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
  JobShop(JobShop &prob);
  JobShop(JobShop &prob, int maq, int pos1, int pos2);

  int calcMakespan();
  void imprimir();
};


int findOrdem(int M, int maq, int* job);

void* alocaMatriz(int, int, int, int);

void desalocaMatriz(int, void*, int, int);

bool fncomp(Problema*, Problema*);

ptrdiff_t myrandom (ptrdiff_t i);

#endif
