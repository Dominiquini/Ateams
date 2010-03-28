/*
  ============================================================================
  Name        : ateams.c
  Author      :
  Version     :
  Copyright   : Your copyright notice
  Description : ATEAMS in C
  ============================================================================
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>

#include "io.h"
#include "memoria.h"
#include "defs.h"
#include "lista.h"
#include "solucoes.h"
#include "politicasMemoria.h"
#include "tabu.h"
#include "genetico.h"
#include "makespan.h"
#include "politicasMemoria.h"

#define PARAMETROS "./parametros/default.param"
#define RESULTADOS "./resultados/out.res"

/* Parametros e Dados */
ParametrosATEAMS *pATEAMS;
ParametrosAG *pAG;
ParametrosBT *pBT;
Dados *dados;

/* Ateams */
void ateams (int *tamanhoMemoriaATEAMS, no **lista, int ****memoriaAG);


/* Retorna a posicao em que o parametro esta em argv, ou -1 se não existir */
int locPar(char **in, int num, char *key)
{
  for(int i = 0; i < num; i++)
    {
      if(!strcmp(in[i], key))
        return i+1;
    }
  return -1;
}

int main (int argc, char *argv[])
{
  /* Declaracoes de inicio para verificacao do uso da memoria dinamica. */
  struct mallinfo info;
  int MemDinInicial, MemDinFinal;
  info = mallinfo();
  MemDinInicial = info.uordblks;

  /* Tempo */
  struct timeval tv1, tv2;
  int s;
  gettimeofday(&tv1, NULL);

  /* Leitura dos parametros passados por linha de comando */
  FILE *fdados;
  FILE *fparametros;
  FILE *fresultados;

  pATEAMS = (ParametrosATEAMS*)malloc(sizeof(ParametrosATEAMS));
  pBT = (ParametrosBT*)malloc(sizeof(ParametrosBT));
  pAG = (ParametrosAG*)malloc(sizeof(ParametrosAG));
  dados = (Dados*)malloc(sizeof(Dados));

  int p = -1;

  if((p = locPar(argv, argc, "-i")) != -1)
    {
      if ((fdados = fopen(argv[p], "r")) == NULL)
        {
          printf("\nArquivo \"%s\" nao encontrado.\n\n", argv[p]);
          exit(1);
        }
      else
        {
          printf("\nDados: '%s'\n", argv[p]);
        }
    }
  else
    {
      printf("\nEscolha um arquivo de entrada: 'ateams -i arq.prb'.\n\n");
      exit(1);
    }

  if((p = locPar(argv, argc, "-p")) != -1)
    {
      if ((fparametros = fopen(argv[p], "r")) == NULL)
        {
          printf("Arquivo \"%s\" nao encontrado.\n\n",argv[p]);
          exit(1);
        }
      else
        {
          printf("Parametros: '%s'\n", argv[p]);
        }
    }
  else
    {
      fparametros = fopen(PARAMETROS, "r");
      printf("Parametros: '%s'\n", PARAMETROS);
    }

  if((p = locPar(argv, argc, "-r")) != -1)
    {
      if ((fresultados = fopen(argv[p], "a")) == NULL)
        {
          printf("Arquivo \"%s\" nao encontrado.\n\n",argv[p]);
          exit(1);
        }
      else
        {
          printf("Resultados: '%s'\n", argv[p]);
        }
    }
  else
    {
      fresultados = fopen(RESULTADOS, "a");
      printf("Resultado: '%s'\n", RESULTADOS);
    }

  if((p = locPar(argv, argc, "-b")) != -1)
    pATEAMS->makespanBest = atoi(argv[p]);
  else
    pATEAMS->makespanBest = -1;

  /* Leitura dos arquivos de dados e de parametros */
  int tipoArquivoDados = 0;                                   //Orlib
  lerArquivoDados(tipoArquivoDados, fdados);
  lerArquivoParametros(fparametros);

  fclose(fdados);
  fclose(fparametros);

  if((p = locPar(argv, argc, "-t")) != -1)
    pATEAMS->iteracoesAteams = atoi(argv[p]);

  if((p = locPar(argv, argc, "-a")) != -1)
    pATEAMS->agenteUtilizado = atoi(argv[p]);

  /* Prepara para a execucao do Ateams */
  int tamanhoMemoriaATEAMS;
  no *memoriaATEAMS;
  int ***memoriaAG;
  Mkp *vmkp;
  int *vetprob;
  int makespanInicial;

  inicializaMemoriaATEAMS(&tamanhoMemoriaATEAMS, &memoriaATEAMS);
  populaMemoriaATEAMS(dados, pATEAMS, &tamanhoMemoriaATEAMS, &memoriaATEAMS);
  makespanInicial = memoriaATEAMS->makespan;
  populaMemoriaAG(dados, pATEAMS, pAG, &vmkp, &memoriaAG);
  vetprob = vetorProb(tamanhoMemoriaATEAMS);

  /* Executa o Ateams */
  printf("\n\nMakespan Inicial: %d\n", makespanInicial);

  ateams (&tamanhoMemoriaATEAMS, &memoriaATEAMS, &memoriaAG);

  /* Libera memoria */
  for(int i = 0; i < pAG->tamanhoPopulacao; i++)
    {
      for(int j = 0; j < dados->M; j++)
        {
          free (memoriaAG[i][j]);
        }
      free (memoriaAG[i]);
    }
  free (vmkp);
  free (memoriaAG);

  /* Apresenta os resultados */
  no *aux;
  int **symbolic;
  int ***esc;
  int mksp;

  aux = retornaElemento(memoriaATEAMS, 1, tamanhoMemoriaATEAMS);
  symbolic = aux->sequence;
  mksp = makespan(dados->m_op, dados->t_op, symbolic, dados->N, dados->M, &esc);

  printf("\n\nMakespan Final: %d\n\n", mksp);

  if(mksp > 0)
    liberaVetorDeMatrizes(dados->M, dados->N, esc);

  gettimeofday(&tv2, NULL);
  s = (((tv2.tv_sec*1000)+(tv2.tv_usec/1000)) - ((tv1.tv_sec*1000)+(tv1.tv_usec/1000)))/1000;
  imprimeResultado(tv1, tv2, s, tamanhoMemoriaATEAMS, memoriaATEAMS, fresultados, makespanInicial);

  fclose(fresultados);

  /* TODO: agrupar todas as desalocacoes de memoria */
  liberaLista(memoriaATEAMS, dados->M);
  liberaMatriz(dados->N, dados->m_op);
  liberaMatriz(dados->N, dados->t_op);
  liberaVetorProb(vetprob);

  free(pATEAMS);
  free(pBT);
  free(pAG);
  free(dados);

  /* Verificacao final do uso da memoria dinamica */
  info = mallinfo();
  MemDinFinal = info.uordblks;

  if (MemDinInicial != MemDinFinal)
    printf("\nMEMORIA DINAMICA NAO FOI TOTALMENTE LIBERADA (%d, %d).\n\n", MemDinInicial, MemDinFinal);
  else
    printf("\nMEMORIA LIBERADA.\n\n");

  return 0;
}

void ateams(int *tamanhoMemoriaATEAMS, no **lista, int ****memoriaAG)
{
  int agente;
  int *vetprob;
  no *n;

  vetprob = vetorProb(pATEAMS->tamanhoPopulacao);

  srand(time(NULL));
  printf("\n");

  for(int i = 1; i <= pATEAMS->iteracoesAteams; i++)
    {
      printf("\nIteracoes Ateams: %d\n", i);

      agente = 1 + (int)(100.0*rand()/(RAND_MAX+1.0));
      agente = agente <= pATEAMS->agenteUtilizado ? 1 : 0;

      switch(agente)
        {
        case 0:
          printf("Makespan AG: ");
          genetico(pATEAMS, pAG, dados, tamanhoMemoriaATEAMS, lista, vetprob, memoriaAG);
          break;
        case 1:
          printf("Makespan BT: ");
          tabu(pATEAMS, pBT, dados, tamanhoMemoriaATEAMS, lista, vetprob);
          break;
        default:
          break;
        }

      n = retornaElemento(*lista, 1, *tamanhoMemoriaATEAMS);

      if(n->makespan <= pATEAMS->makespanBest)
        {
          printf("\nPopulacao ATEAMS convergiu na %d iteracao.\n\n", i);
          break;
        }
    }

  free(vetprob);

  return;
}
