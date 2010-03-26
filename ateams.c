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
          printf("Arquivo \"%s\" nao encontrado.\n", argv[p]);
          exit(1);
        }
      else
        {
          printf("Dados: '%s'\n", argv[p]);
        }
    }
  else
    {
      printf("\n\nEscolha um arquivo de entrada: 'ateams -i arq.prb'.\n");
      exit(1);
    }

  if((p = locPar(argv, argc, "-p")) != -1)
    {
      if ((fparametros = fopen(argv[p], "r")) == NULL)
        {
          printf("Arquivo \"%s\" nao encontrado.\n",argv[p]);
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
          printf("Arquivo \"%s\" nao encontrado.\n",argv[p]);
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

  /* Leitura dos arquivos de dados e de parametros */
  int tipoArquivoDados = 0;                                   //Orlib
  lerArquivoDados (tipoArquivoDados, fdados);
  lerArquivoParametros (fparametros);

  /* Prepara para a execucao do Ateams */
  int tamanhoMemoriaATEAMS;
  no *memoriaATEAMS;
  int ***memoriaAG;
  Mkp *vmkp;
  int *vetprob;
  int makespanInicial;

  inicializaMemoriaATEAMS (&tamanhoMemoriaATEAMS, &memoriaATEAMS);
  populaMemoriaATEAMS (dados, pATEAMS, &tamanhoMemoriaATEAMS, &memoriaATEAMS);
  makespanInicial = memoriaATEAMS->makespan;
  populaMemoriaAG (dados, pATEAMS, pAG, &vmkp, &memoriaAG);
  vetprob = vetorProb(tamanhoMemoriaATEAMS);

  /* Executa o Ateams */
  ateams (&tamanhoMemoriaATEAMS, &memoriaATEAMS, &memoriaAG);

  /* Libera memoria */
  for (int i = 0; i < pAG->tamanhoPopulacao; i++) {
    for (int j = 0; j < dados->M; j++) {
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

  printf("%d\n", mksp);

  if (mksp > 0)
    liberaVetorDeMatrizes (dados->M, dados->N, esc);

  gettimeofday(&tv2,NULL);
  s = (((tv2.tv_sec*1000)+(tv2.tv_usec/1000)) - ((tv1.tv_sec*1000)+(tv1.tv_usec/1000)))/1000;
  imprimeResultado (tv1, tv2, s, tamanhoMemoriaATEAMS, memoriaATEAMS, fresultados, makespanInicial);

  /* TODO: agrupar todas as desalocacoes de memoria */
  liberaLista (memoriaATEAMS, dados->M);
  liberaMatriz(dados->N, dados->m_op);
  liberaMatriz(dados->N, dados->t_op);
  liberaVetorProb (vetprob);

  free(pATEAMS);
  free(pBT);
  free(pAG);
  free(dados);

  /* Verificacao final do uso da memoria dinamica */
  info = mallinfo();
  MemDinFinal = info.uordblks;
  if (MemDinInicial!=MemDinFinal)
    printf("\n\nMemoria dinamica nao foi totalmente liberada (%d, %d).\n\n", MemDinInicial, MemDinFinal);
  else
    printf("\n\nMEMORIA LIBERADA.\n\n");

  return 0;
}

void ateams (int *tamanhoMemoriaATEAMS, no **lista, int ****memoriaAG)
{
  int i;
  int agente;
  int *vetprob;
  no *n1;//, *n2;
  int maxSemMelhoria;
  FILE *f = fopen ("lawrence.best", "r");
  if (f == NULL) {
    perror("resultados\\lawrence.best");
    exit(1);
  }

  int *bestMakespan = NULL;

  if (pATEAMS->tamanhoPopulacao < 5) {
    maxSemMelhoria = pATEAMS->tamanhoPopulacao;
  }
  else
    maxSemMelhoria = 5;

  vetprob = vetorProb (pATEAMS->tamanhoPopulacao);

  //genetico (pATEAMS, pAG, dados, tamanhoMemoriaATEAMS, lista, vetprob);
  //tabu (pATEAMS, pBT, dados, tamanhoMemoriaATEAMS, vetprob, lista);


  bestMakespan = lerArquivoBestMakespan (f, 40);

  pATEAMS->makespanBest = bestMakespan[dados->instancia - 1];
  if (pATEAMS->agenteUtilizado == ATEAMS_ON)
    for (i = 0; i < pATEAMS->iteracoesAteams; i++)
      {
        printf("Iteracoes Ateams: %d\n", i);
        agente = rand () % 2;
        switch (agente) {
        case 0:
          genetico (pATEAMS, pAG, dados, tamanhoMemoriaATEAMS, lista, vetprob, memoriaAG);
          break;
        case 1:
          tabu (pATEAMS, pBT, dados, tamanhoMemoriaATEAMS, lista, vetprob);
          break;
        default:
          break;
        }
        n1 = retornaElemento(*lista, 1, *tamanhoMemoriaATEAMS);
        //n2 = retornaElemento(*lista, maxSemMelhoria, *tamanhoMemoriaATEAMS);
        //printf("makespan melhor memoria ATEAMS: %d %d\n", n1->makespan, bestMakespan[dados->instancia-1]);
        if (n1->makespan == pATEAMS->makespanBest) {
          printf("Populacao ATEAMS convergiu na %d iteracao.\n", 1+i);
          break;
        }
      }
  else
    for (i = 0; i < pATEAMS->iteracoesAteams; i++)
      {
        //printf("Iteracoes Ateams: %d\n", i);
        switch (pATEAMS->agenteUtilizado) {
        case 0:
          genetico (pATEAMS, pAG, dados, tamanhoMemoriaATEAMS, lista, vetprob, memoriaAG);
          break;
        case 1:
          tabu (pATEAMS, pBT, dados, tamanhoMemoriaATEAMS, lista, vetprob);
          break;
        default:
          break;
        }
        /*n1 = retornaElemento(*lista, 1, *tamanhoMemoriaATEAMS);
          n2 = retornaElemento(*lista, pATEAMS->tamanhoPopulacao, *tamanhoMemoriaATEAMS);
          if (n1->makespan == n2->makespan) {
          printf("Populacao ATEAMS convergiu na %d iteracao.\n", 1+i);
          break;
          }*/
      }
  fclose (f);
  free (bestMakespan);
  free (vetprob);
}
