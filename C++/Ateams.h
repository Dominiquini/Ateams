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
  float agenteUtilizado;
  int tamanhoPopulacao;   /* > 0*/
  int iteracoesAteams;
  int maxTempo;
  int politicaAceitacao;
  int politicaDestruicao;
  int makespanBest;       /* Melhor makespan conhecido */
} ParametrosATEAMS;

typedef struct ParametrosAG {
  int selecao;
  int quantidadeLeituraMemoriaATEAMS;
  int tamanhoPopulacao;
  int numeroIteracoes;
  int politicaLeitura;
  float probabilidadeCrossover;
  float probabilidadeMutacoes;
} ParametrosAG;

typedef struct ParametrosBT {
  int politicaLeitura;
  int numeroIteracoes;
  int tamanhoListaTabu;
  int k;                /* valor que ira determinar o tamanho da lista tabu de acordo como o numero de iteracoes */
} ParametrosBT;

#endif
