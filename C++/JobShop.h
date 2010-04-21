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

  ~JobShop();

  int calcMakespan();
  void imprimir();
};

void* alocaMatriz(int, int, int, int);

void desalocaMatriz(int, void*, int, int);

int findOrdem(int M, int maq, int* job);

ptrdiff_t myrandom (ptrdiff_t i);
bool fncomp(Problema*, Problema*);

#endif
