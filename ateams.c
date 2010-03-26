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



void ateams (ParametrosATEAMS *pATEAMS, ParametrosAG *pAG, ParametrosBT *pBT, Dados *dados, int *tamanhoMemoriaATEAMS, no **lista, int ****memoriaAG);

int main (int argc, char *argv[])
{
  int i, j;
  int tamanhoMemoriaATEAMS;
  no *memoriaATEAMS;
  int ***memoriaAG;

  ParametrosATEAMS pATEAMS;
  ParametrosAG pAG;
  ParametrosBT pBT;
  Dados dados;
  Mkp *vmkp;
  int *vetprob;

  struct timeval tv1, tv2; /* tempo */
  int s;         /* tempo */

  /* Serão lidos na entrada do programa */
  int tipoArquivoDados = 0;

  int makespanInicial;
  no *aux;

  FILE *fdados;
  FILE *fparametros;
  FILE *fresultados;

  /* Declarações de início para verificação do uso da memória dinâmica. */
  struct mallinfo info;
  int MemDinInicial, MemDinFinal;
  info = mallinfo();
  MemDinInicial = info.uordblks;

  gettimeofday(&tv1,NULL);
  setParametrosATEAMS (&pATEAMS);
  setParametrosAG (&pAG);
  setParametrosBT (&pBT);

  leArgumentos(argc, argv, &fdados, &fparametros, &fresultados, &tipoArquivoDados);

  lerArquivoDados (tipoArquivoDados, fdados, &dados);
  lerArquivoParametros (fparametros, &pATEAMS, &pAG, &pBT);
  inicializaMemoriaATEAMS (&tamanhoMemoriaATEAMS, &memoriaATEAMS);
  populaMemoriaATEAMS (&dados, &pATEAMS, &tamanhoMemoriaATEAMS, &memoriaATEAMS);
  makespanInicial = memoriaATEAMS->makespan;
  populaMemoriaAG (&dados, &pATEAMS, &pAG, &vmkp, &memoriaAG);
  vetprob = vetorProb(tamanhoMemoriaATEAMS);


  dados.instancia = atoi (argv[5]);
  ateams (&pATEAMS, &pAG, &pBT, &dados, &tamanhoMemoriaATEAMS, &memoriaATEAMS, &memoriaAG);

  for (i = 0; i < pAG.tamanhoPopulacao; i++) {
    for (j = 0; j < dados.M; j++) {
      free (memoriaAG[i][j]);
    }
    free (memoriaAG[i]);
  }
  free (vmkp);
  free (memoriaAG);

  int **symbolic;
  int ***esc;
  int mksp;
  aux = retornaElemento(memoriaATEAMS, 1, tamanhoMemoriaATEAMS);
  symbolic = aux->sequence;
  mksp = makespan(dados.m_op, dados.t_op, symbolic, dados.N, dados.M, &esc);
  printf("%d\n", mksp);/*
                         if (M < 10 && N < 10 && mksp != 0)
                         imprimir(N, M, esc);*/
  if (mksp > 0) liberaVetorDeMatrizes (dados.M, dados.N, esc);

  gettimeofday(&tv2,NULL);
  s = (((tv2.tv_sec*1000)+(tv2.tv_usec/1000)) - ((tv1.tv_sec*1000)+(tv1.tv_usec/1000)))/1000;
  imprimeResultado (tv1, tv2, s, tamanhoMemoriaATEAMS, memoriaATEAMS, fresultados, makespanInicial);

  /* TODO: agrupar todas as desalocações de memória */
  liberaLista (memoriaATEAMS, dados.M);
  liberaMatriz(dados.N, dados.m_op);
  liberaMatriz(dados.N, dados.t_op);
  liberaVetorProb (vetprob);


  /* Verificacao final do uso da memoria dinamica */
  info = mallinfo();
  MemDinFinal = info.uordblks;
  if (MemDinInicial!=MemDinFinal)
    printf("\n\nMemoria dinamica nao foi totalmente liberada (%d, %d).\n",
           MemDinInicial,MemDinFinal);
  else
    printf("\n\nMEMORIA LIBERADA.\n\n");
  /*----------------------------------------------------------------------- */
  return 0;
}

void ateams (ParametrosATEAMS *pATEAMS, ParametrosAG *pAG, ParametrosBT *pBT, Dados *dados, int *tamanhoMemoriaATEAMS, no **lista, int ****memoriaAG)
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

  pATEAMS->makespanBest = bestMakespan[dados->instancia-1];
  if (pATEAMS->agenteUtilizado == ATEAMS_ON)
    for (i = 0; i < pATEAMS->iteracoesAteams; i++) {
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
    for (i = 0; i < pATEAMS->iteracoesAteams; i++) {
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
