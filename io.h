/*
 * io.h
 *
 *  Created on: 02/03/2010
 *      Author: iha
 */

#ifndef IO_H_
#define IO_H_
#include <stdio.h>
#include <sys/time.h>
#include "lista.h"

typedef struct ParametrosATEAMS {
  int agenteUtilizado;
  int tamanhoPopulacao;   /* > 0*/
  int iteracoesAteams;
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

typedef struct TamanhoTabu {
  int atual;
  int min;
  int max;
} TamanhoTabu;

typedef struct Makespan {
  int atual;
  int melhor;
  int anterior;
  int iteracoesSemMelhoria;
  int maxSemMelhoria;
} Makespan;

typedef struct ParametrosBT {
  int politicaLeitura;
  int numeroIteracoes;
  int tamanhoListaTabu;
  TamanhoTabu tamanho;
  Makespan makespan;
  int k;                /* valor que ira determinar o tamanho da lista tabu de acordo como o numero de iteracoes */
} ParametrosBT;

typedef struct Dados {
  char nome[100]; /* Nome do problema */
  int instancia;  /* Numero da instancia */
  int N;	  /* Numero de jobs */
  int M;	  /* Numero de maquinas */
  int **m_op;	  /* sequencia de maquinas em cada job */
  int **t_op;	  /* tempo de execucao das tarefas na maquina correspondente */
} Dados;


void menuAteams ();
int menuConfigurarATEAMS (ParametrosATEAMS *p);
int menuConfigurarAG (ParametrosAG *p);
int menuConfigurarBT (ParametrosBT *p);

void leArgumentos (int argc, char *argv[], FILE **dados, FILE **parametros, FILE **resultados, int *tipoArquivoDados);

void escreveArquivoParametros (char nomeArquivo[100], ParametrosATEAMS *pATEAMS, ParametrosAG *AG, ParametrosBT *pBT);

void setParametrosATEAMS (ParametrosATEAMS *p);
void setParametrosAG (ParametrosAG *p);
void setParametrosBT (ParametrosBT *p);

void lerArquivoConfiguracao (char arquivo[100], ParametrosATEAMS *pATEAMS);

void lerArquivoDados (int tipoArquivo, FILE *fdados);
void lerArquivoParametros (FILE *f);

void imprimeResultado (struct timeval tv1, struct timeval tv2, int s, int msize, no *lista, FILE *resultados, int makespanInicial);

int expressaoRegularCasa (char *strA, char *strB);

void mostraMatriz (int linhas, int colunas, int **M);

int *lerArquivoBestMakespan (FILE *f, int tamanho);

#endif /* IO_H_ */
