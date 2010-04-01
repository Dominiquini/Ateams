/*
 * io.c
 *
 *  Created on: 01/03/2010
 *      Author: iha
 */

#include "io.h"
#include "memoria.h"
#include "defs.h"
#include "lista.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#define FALSE 0
#define TRUE 1

#define ORLIB 0
#define TAILLARD 1


extern ParametrosATEAMS *pATEAMS;
extern ParametrosAG *pAG;
extern ParametrosBT *pBT;
extern Dados *dados;



char* locPosPar(char *in, int num, char *key)
{
  char *str = strstr(in, key);

  if(str != NULL)
    return strstr(str, "=") + 1;
  else
    return NULL;
}

float locNumberPar(char *in, int num, char *key)
{
  char *str = locPosPar(in, num, key);
  float ret = -1;

  if(str != NULL)
    sscanf(str, "%f", &ret);

  return ret;
}

void lerArquivoDados (int tipoArquivo, FILE *f)
{
  int i, j;

  if (tipoArquivo == ORLIB) {
    if (!fgets (dados->nome, 100, f))
      exit(1);
    if (!fscanf (f, "%d %d", &dados->N, &dados->M))
      exit(1);
    dados->m_op = alocaMatriz (dados->N, dados->M);
    dados->t_op = alocaMatriz (dados->N, dados->M);
    for (i = 0; i < dados->N; i++) {
      for (j = 0; j < dados->M; j++) {
        if (!fscanf (f, "%d %d", &dados->m_op[i][j], &dados->t_op[i][j]))
          exit(1);
      }
    }
  }
  else if (tipoArquivo == TAILLARD) {
    /* TODO: fazer a leitura de dados taillard */
  }
  else {
    printf("Tipo de arquivo nao definido.\n");
    /* TODO: melhorar a mensagem.*/
    exit(1);
  }
}

void lerArquivoParametros (FILE *f)
{
  char *parametros = (char*)malloc(4097 * sizeof(char));
  size_t size = fread(parametros, sizeof(char), 4096, f);
  float par = -1;

  float porcentagemPop;
  float porcentagemLeituraATEAMS;

  par = locNumberPar(parametros, size, "[agUtilizado]");
  pATEAMS->agenteUtilizado = par != -1 ? par : 0.5;

  par = locNumberPar(parametros, size, "[iterAteams]");
  pATEAMS->iteracoesAteams = par != -1 ? (int)par : 100;

  par = locNumberPar(parametros, size, "[MaxTempo]");
  pATEAMS->maxTempo = par;

  par = locNumberPar(parametros, size, "[polAceitacao]");
  pATEAMS->politicaAceitacao = par != -1 ? (int)par : 2;

  par = locNumberPar(parametros, size, "[polDestruicao]");
  pATEAMS->politicaDestruicao = par != -1 ? (int)par : 1;

  par = locNumberPar(parametros, size, "[tamPopulacao]");
  pATEAMS->tamanhoPopulacao = par != -1 ? (int)par : 50;

  par = locNumberPar(parametros, size, "[makespanBest]");
  pATEAMS->makespanBest = par;


  par = locNumberPar(parametros, size, "[iterAG]");
  pAG->numeroIteracoes = par != -1 ? (int)par : 1000;

  par = locNumberPar(parametros, size, "[polLeituraAG]");
  pAG->politicaLeitura = par != -1 ? (int)par : 1;

  par = locNumberPar(parametros, size, "[%Leitura]");
  porcentagemPop = par != -1 ? par : 0.8;

  par = locNumberPar(parametros, size, "[%Populacao]");
  porcentagemLeituraATEAMS = par != -1 ? par : 1.0;

  par = locNumberPar(parametros, size, "[probCrossover]");
  pAG->probabilidadeCrossover = par != -1 ? par : 0.6;

  par = locNumberPar(parametros, size, "[probMutacao]");
  pAG->probabilidadeMutacoes = par != -1 ? par : 0.02;

  par = locNumberPar(parametros, size, "[Selecao]");
  pAG->selecao = par != -1 ? (int)par : 1;


  par = locNumberPar(parametros, size, "[iterBT]");
  pBT->numeroIteracoes = par != -1 ? (int)par : 1000;

  par = locNumberPar(parametros, size, "[polLeituraBT]");
  pBT->politicaLeitura = par != -1 ? (int)par : 1;

  par = locNumberPar(parametros, size, "[tamListaBT]");
  pBT->tamanhoListaTabu = par != -1 ? (int)par : 2;


  pAG->tamanhoPopulacao = pATEAMS->tamanhoPopulacao * (1 + porcentagemPop);

  pAG->quantidadeLeituraMemoriaATEAMS = pATEAMS->tamanhoPopulacao * porcentagemLeituraATEAMS;

  pBT->k = pBT->numeroIteracoes/5;

  free(parametros);
}

void imprimeResultado (struct timeval tv1, struct timeval tv2, int s, int msize, no *lista, FILE *resultados, int makespanInicial)
{
  no *aux;

  aux = retornaElemento(lista, 1, msize);
  fprintf(resultados,"%8d %8d %8d\n",makespanInicial, aux->makespan, s);

  /*fprintf (indiv,"%7d m %2d s %3d ms\n",min,s,ms); *//* tempo processamento */
}

/* int *lerArquivoBestMakespan (FILE *f, int tamanho) */
/* { */
/*   int *best; */
/*   int i; */

/*   best = (int*) malloc (tamanho * sizeof(int)); */

/*   for (i = 0; i < tamanho; i++) */
/*     fscanf (f, "%d\n", &best[i]); */

/*   return best; */
/* } */

void mostraMatriz (int linhas, int colunas, int **M)
{
  int i, j;
  for (i = 0; i < linhas; i++) {
    for (j = 0; j < colunas; j++)
      printf("%5d", M[i][j]);
    printf("\n");
  }
  printf("\n");
}
