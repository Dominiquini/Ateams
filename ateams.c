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
#include <limits.h>
#include <signal.h>
#include <time.h>

#include "io.h"
#include "memoria.h"
#include "defs.h"
#include "lista.h"
#include "solucoes.h"
#include "tabu.h"
#include "genetico.h"
#include "makespan.h"

#define PARAMETROS "parametros/default.param"

/* Indica ao programa para parar a execucao */
int PARAR = 0;

/* Parametros e Dados */
ParametrosATEAMS *pATEAMS;
ParametrosAG *pAG;
ParametrosBT *pBT;
Dados *dados;

/* Ateams */
void ateams (int *tamanhoMemoriaATEAMS, no **lista, int ****memoriaAG);

void Interrompe(int signum)
{
  PARAR = 1;
}

int main (int argc, char *argv[])
{
  /* Interrompe o programa ao se pessionar 'Ctrl-c' */
  signal(SIGINT, Interrompe);

  /* Verificacao do uso da memoria dinamica. */
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

  if((p = locComPar(argv, argc, "-i")) != -1)
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

  if((p = locComPar(argv, argc, "-p")) != -1)
    {
      if ((fparametros = fopen(argv[p], "r")) == NULL)
        {
          printf("Arquivo \"%s\" nao encontrado.\n\n", argv[p]);
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

  if((p = locComPar(argv, argc, "-r")) != -1)
    {
      if ((fresultados = fopen(argv[p], "a")) == NULL)
        {
          printf("Arquivo \"%s\" nao encontrado.\n\n", argv[p]);
          exit(1);
        }
      else
        {
          printf("Resultados: '%s'\n", argv[p]);
        }
    }
  else
    {
      p = locComPar(argv, argc, "-i");

      char resultado[32] = {"resultados/"};
      strcat(resultado, strstr(argv[p], "dados/") + 6);
      resultado[strlen(resultado) - 3] = '\0';
      strcat(resultado, "res");

      fresultados = fopen(resultado, "a");
      printf("Resultado: '%s'\n", resultado);
    }

  /* Leitura dos arquivos de dados e de parametros */
  int tipoArquivoDados = 0;  //Orlib

  lerArquivoDados(tipoArquivoDados, fdados);
  lerArquivoParametros(fparametros);


  fclose(fdados);
  fclose(fparametros);


  lerArgumentos(argv, argc);


  pATEAMS->maxTempo = pATEAMS->maxTempo <= 0 ? INT_MAX : pATEAMS->maxTempo;

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

  ateams(&tamanhoMemoriaATEAMS, &memoriaATEAMS, &memoriaAG);

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

  struct timeval time1, time2;
  gettimeofday(&time1, NULL);

  int tempo = 0;

  pATEAMS->agenteUtilizado = pATEAMS->agenteUtilizado * 100;

  for(int i = 1; i <= pATEAMS->iteracoesAteams && tempo < pATEAMS->maxTempo; i++)
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
          printf("\n\nPopulacao ATEAMS convergiu na %d iteracao.\n", i);
          break;
        }

      if(PARAR == 1)
        break;

      gettimeofday(&time2, NULL);
      tempo = time2.tv_sec - time1.tv_sec;
    }

  free(vetprob);

  return;
}
