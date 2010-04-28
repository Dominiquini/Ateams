#include <signal.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

using namespace std;

#define ESCALONAMENTO false

#ifndef _ATEAMS_
#define _ATEAMS_

typedef struct soluction
{
	int makespan;	// Makespan da solucao
	int **esc;		// Solucao
	int ***escalon;	// Escalonamento nas maquinas
} resp;

typedef struct movTabu
{
	int maq, A, B;
	bool job;
} mov;

typedef struct ParametrosATEAMS {
	int tamanhoPopulacao;
	int iteracoesAteams;
	int maxTempo;
	int makespanBest;       /* Melhor makespan conhecido */
} ParametrosATEAMS;

typedef struct ParametrosAG {
	// TODO
} ParametrosAG;

typedef struct ParametrosBT {
	float funcAsp;
	int probBT;
	int polEscolha;
	int tentativasSemMelhora;
	int numeroIteracoes;
	int tamanhoListaTabu;
} ParametrosBT;

#endif
