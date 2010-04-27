#include <signal.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

using namespace std;

#ifndef _ATEAMS_
#define _ATEAMS_

typedef struct ParametrosATEAMS {
	int tamanhoPopulacao;
	int iteracoesAteams;
	int maxTempo;
	int makespanBest;       /* Melhor makespan conhecido */
} ParametrosATEAMS;

typedef struct ParametrosAG {
	int quantidadeLeituraMemoriaATEAMS;
	int tamanhoPopulacao;
	int numeroIteracoes;
	int politicaLeitura;
	float probabilidadeCrossover;
	float probabilidadeMutacoes;
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
