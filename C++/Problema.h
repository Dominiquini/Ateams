#include <stdio.h>

#include <functional>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <list>
#include <set>

#include "Ateams.h"

using namespace std;

#ifndef _PROBLEMA_
#define _PROBLEMA_

class Problema;

bool fncomp(Problema*, Problema*);


class Problema
{
protected:
	soluction sol;	// Makespan e escalonamentos que definem a solucao
	mov movTabu;	// Movimento tabu que gerou a solucao. movTabu.maq = -1 se por outro meio

public:
	static int numInst;				// Quantidade de instancias criadas
	static double totalMakespan;	// Soma do inverso do makespan de todos os individuos na populacao

	// Le arquivo de dados de entrada
	static void leProblema(FILE*);

	// Le arquivo de parametros de entrada
	static void leParametros(FILE*, ParametrosATEAMS*, ParametrosBT*, ParametrosAG*);
	static void leArgumentos(char**, int, ParametrosATEAMS*, ParametrosBT*, ParametrosAG*);

	static void imprimeResultado (struct timeval, struct timeval, FILE*, int);

	static void desalocaMemoria();

	static Problema* alloc();												// Nova solucao aleatoria
	static Problema* alloc(int **prob);										// Copia de prob
	static Problema* alloc(Problema &prob);									// Copia de prob
	static Problema* alloc(Problema &prob, int maq, int pos1, int pos2);	// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	static bool movTabuCMP(mov& t1, mov& t2);
	static double sumFitness(set<Problema*, bool(*)(Problema*, Problema*)> *pop, int n);

	Problema();		// numInst++

	~Problema();	// numInst--

	virtual int calcMakespan() = 0;	// Calcula o makespan
	virtual void imprimir() = 0;	// Imprime o escalonamento

	/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual */
	virtual multiset<Problema*, bool(*)(Problema*, Problema*)>* buscaLocal() = 0;

	virtual double getFitness() = 0;
	virtual int getMakespan() = 0;

	friend class JobShop;
	friend class Tabu;
	friend bool fncomp(Problema*, Problema*);
};

#endif
