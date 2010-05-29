#include <signal.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

using namespace std;

// #define ESCALONAMENTO

#define THREADS

#ifndef _ATEAMS_
#define _ATEAMS_

typedef struct soluction
{
	int makespan;			// Makespan da solucao
	short int **esc;		// Solucao
	short int ***escalon;	// Escalonamento nas maquinas
} soluction;

typedef struct executado
{
	bool tabu;
	bool genetico;
	bool annealing;
} executado;

typedef struct movTabu
{
	short int maq, A, B;
} tTabu;

typedef struct ParametrosATEAMS {
	int tamanhoPopulacao;
	int iteracoesAteams;
	int numThreads;
	int maxTempo;
	int makespanBest;
} ParametrosATEAMS;

typedef struct ParametrosAG {
	int probAG;
	int polEscolha;
	int numeroIteracoes;
	int tamanhoPopulacao;
	int tamanhoAuxPopulacao;
	int tamanhoParticionamento;
	float probCrossOver;
	float probMutacao;
} ParametrosAG;

typedef struct ParametrosBT {
	int probBT;
	int polEscolha;
	int tentativasSemMelhora;
	int numeroIteracoes;
	int tamanhoListaTabu;
	float funcAsp;
} ParametrosBT;

typedef struct ParametrosSA {
	int probSA;
	int polEscolha;
	int maxIter;
	int initTemp;
	int fimTemp;
	float alfa;
} ParametrosSA;

#endif
