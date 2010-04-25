#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "Problema.h"

using namespace std;

#ifndef _JobShop_
#define _JobShop_

class JobShop : public Problema
{
public:

	JobShop();												// Nova solucao aleatoria
	JobShop(int **prob);									// Copia de prob
	JobShop(Problema &prob);								// Copia de prob
	JobShop(Problema &prob, int maq, int pos1, int pos2);	// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	int calcMakespan();	// Calcula o makespan
	void imprimir();	// Imprime o escaloonamento atual

	/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual */
	multiset<Problema*, bool(*)(Problema*, Problema*)>* buscaLocal();

	double getFitness();
};

float locNumberPar(char *in, int num, char *key);

char* locPosPar(char *in, int num, char *key);

int findOrdem(int M, int maq, int* job);

void* alocaMatriz(int, int, int, int);

void desalocaMatriz(int, void*, int, int);

bool fncomp(Problema*, Problema*);

ptrdiff_t myrandom (ptrdiff_t i);

#endif
