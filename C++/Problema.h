#include <stdio.h>

#include <functional>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <list>
#include <set>

using namespace std;

#ifndef _PROBLEMA_
#define _PROBLEMA_


typedef struct movTabu
{
	int maq, A, B;
} mov;

class Problema;

bool fncomp(Problema*, Problema*);

class Problema
{
public:
	static int numInst;				// Quantidade de instancias criadas

	static char name[128];			// Nome do problema
	static int **maq, **time;		// Matriz de maquinas e de tempos
	static int njob, nmaq;			// QUantidade de jobs e de maquinas

	static void leProblema(FILE*);	// Le arquivo de dados de entrada


	mov movTabu;	// Movimento tabu que gerou a solucao. movTabu.maq = -1 se por outro meio

	int **esc;		// Solucao
	int makespan;	// Makespan da solucao
	int ***escalon;	// Escalonamento nas maquinas

	Problema();		// numInst++

	~Problema();	// numInst--

	virtual int calcMakespan() = 0;	// Calcula o makespan
	virtual void imprimir() = 0;	// Imprime o escalonamento

	/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual */
	virtual multiset<Problema*, bool(*)(Problema*, Problema*)>* buscaLocal() = 0;
};

#endif
