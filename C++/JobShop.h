#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#include "Problema.h"

using namespace std;

#ifndef _JobShop_
#define _JobShop_

class JobShop : public Problema
{
public:
	static char name[128];			// Nome do problema
	static short int **maq, **time;	// Matriz de maquinas e de tempos
	static int njob, nmaq;			// Quantidade de jobs e de maquinas

	static tTabu* newTabu(int maq, int p1, int p2);

	JobShop();												// Nova solucao aleatoria
	JobShop(short int **prob);								// Copia de prob
	JobShop(const Problema &prob);								// Copia de prob
	JobShop(const Problema &prob, int maq, int pos1, int pos2);	// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	~JobShop();

	int calcMakespan();			// Calcula o makespan
	void imprimir(bool esc);	// Imprime o escaloonamento atual

	/* Retorna um novo vizinho aleatorio */
	Problema* vizinho();

	/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual. Retorna 'n' novos indivíduos */
	vector<pair<Problema*, tTabu*>* >* buscaLocal();

	/* Faz o crossover da solucao atual com a passada como parametro. Retorna dois novos individuos */
	pair<Problema*, Problema*>* crossOver(Problema*, int);
	pair<Problema*, Problema*>* crossOver(Problema*);

	/* Provoca uma mutacao na solucao atual */
	void mutacao();

	double getFitnessMaximize();
	double getFitnessMinimize();

	int** getEscalonameto();
};

void swap_vect(short int* p1, short int* p2, short int* f, int pos, int tam);

int locComPar(char **in, int num, char *key);

float locNumberPar(char *in, int num, char *key);

char* locPosPar(char *in, int num, char *key);

int findOrdem(int M, int maq, short int* job);

void* alocaMatriz(int, int, int, int);

void desalocaMatriz(int, void*, int, int);

bool ptcomp(pair<Problema*, tTabu*>*, pair<Problema*, tTabu*>*);

bool ppcomp(pair<Problema*, Problema*>* p1, pair<Problema*, Problema*>* p2);

#endif
