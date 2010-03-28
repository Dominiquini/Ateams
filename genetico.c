/*
 * genetico.c
 *
 *  Created on: 06/03/2010
 *      Author: iha
 */

#include "makespan.h"
#include "solucoes.h"
#include "memoria.h"
#include "genetico.h"
#include "defs.h"
#include "lista.h"
#include "io.h"
#include "politicasMemoria.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "utils.h"

/* Inicializa parametros do AG usado
   se a versão for paralelizada é necessario fazer isso localmente no AG
   por enquanto as variaves sao globais
*/

/*
 * Funcoes auxiliares: Apenas para desenvolvimento
 */
void imprimeSequenciaGenetico (int **seq, int linhas, int colunas) {
  int i, j;

  printf("\nSequencia AG:\n");
  for (i = 0; i < linhas; i++) {
    for (j = 0; j < colunas; j++)
      printf("%3d", seq[i][j]);
    printf("\n");
  }
}
/*
 * FIM das funcoes auxiliares: Apenas para desenvolvimento
 */

void populaMemoriaAG (Dados *d, ParametrosATEAMS *pATEAMS, ParametrosAG *pAG, Mkp **vmkp, int ****memoriaAG)
{
  int i, j;

  *vmkp = (Mkp*) malloc (pAG->tamanhoPopulacao * sizeof(Mkp));

  *memoriaAG = (int ***) malloc (pAG->tamanhoPopulacao * sizeof(int **));

  for (i = 0; i < pAG->tamanhoPopulacao; i++) {
    (*memoriaAG)[i] = (int **) malloc(d->M * sizeof(int *));
    for (j = 0; j < d->M; j++)
      (*memoriaAG)[i][j] = (int *) malloc(d->N * sizeof(int));
    (*vmkp)[i].ok = 0;
  }
  for (i = 0; i < pAG->tamanhoPopulacao; i++) {
    (*vmkp)[i].mkp = geraSolucaoAleatoria((*memoriaAG)[i], d);
    (*vmkp)[i].ok = 1;
  }
}


/*
 * Funcao auxiliar do quicksort
 */
int compare_record(const void *a, const void *b)
{
  mkord *x = (mkord *)a;
  mkord *y = (mkord *)b;
  return(x->valor - y->valor);
}

/*
 * Funcao usada para ordenar a populacao pelo Makespan
 */
int ordenaPopulacao (mkord *vaux, int tamanhoPopulacao, Mkp *vmkp, int ***pop, int ***popaux)
{
  int i;
  /* Estrutura auxiliar para fazer a ordenacao */
  for (i = 0; i < tamanhoPopulacao; i++) {
    vaux[i].valor = vmkp[i].mkp;
    vaux[i].posicao = i;
  }
  qsort (vaux, tamanhoPopulacao, sizeof(mkord), compare_record);
  for (i = 0; i < tamanhoPopulacao; i++) {
    popaux[i] = pop[vaux[i].posicao];
    vmkp[i].mkp = vaux[i].valor;
    vmkp[i].ok = 1;
  }
  for (i = 0; i < tamanhoPopulacao; i++) /* Só para facilitar o uso do gdb */
    pop[i] = NULL;
  return (sizeof(mkord)*tamanhoPopulacao);
}


/*
 * Escreve a melhor solucao encontrada na Memoria Principal
 */
void escreveGenetico (Dados *dados, ParametrosATEAMS *pATEAMS, int *msize, int *vetprob, no **lista, int **solucao, Mkp *vmkp)
{
  int i, j;
  int **seqaux;

  seqaux = (int**) malloc(dados->M * sizeof(int*));
  for (i = 0; i < dados->M; i++)
    seqaux[i] = (int*) malloc(dados->N * sizeof(int));

  for (i = 0; i < dados->M; i++) {
    for (j = 0; j < dados->N; j++)
      seqaux[i][j] = solucao[i][j];
  }
  politicaAceitacao (seqaux, vmkp[0].mkp, dados, pATEAMS, msize, vetprob, lista);
  printf("%d.\n", vmkp[0].mkp);
}

/* Seleciona uma solucao da populacao do algoritmo genetico.
 * Antes da primeira chamada de selecaoAG(), melhorSolucao deve ser setada com zero */
#if 0
no *selecaoAG (ParametrosAG *pAG, int ***pop, int *vetprob, int *melhorSolucao)
{
  int j;
  no *solucao;
  int msize = pAG->tamanhoPopulacaoFinal;
  switch (pAG->politicaLeitura) {
  case 1: /* escolhe a melhor solucao */
    solucao = retornaElemento (principal, *melhorSolucao, msize);
    melhorSolucao++;
    break;
  case 2: /* escolhe a sequencias randomicamente[Probabilidade Uniforme] */
    j = (rand() % msize) + 1;
    solucao = retornaElemento (principal, j, msize);
    break;
  case 3: /* Lê np sequencias com Probabilidade Linear da melhor para a pior */
    j = (1 + msize) - sequenciaProbLinear (msize, vetprob);
    solucao = retornaElemento (principal, j, msize);
    break;
  case 4: /* Lê np sequencias com Probabilidade Linear da pior para a melhor */
    j = sequenciaProbLinear (msize, vetprob);
    solucao = retornaElemento (principal, j, msize);
    break;
  case 5: /* Lê np sequencias randomicamente[Probabilidade Uniforme] da pior para melhor */
    j = (rand() % msize) + 1;
    solucao = retornaElemento (principal, msize-j, msize);
    break;
  default:
    printf("Politica de leitura nao definida: %d.\n", pAG->politicaLeitura);
    break;
  }

}
#endif


/* Reinicializa a memória do AG com uma porcentagem da memória principal */
void  leituraMemoriaATEAMS (Dados *dados, ParametrosATEAMS *pATEAMS, ParametrosAG *pAG, int msize, no *principal, int ****pop, Mkp *vmkp, int *vetprob)
{
  int i, j, k, l = 1;
  no *aux = NULL;

  //
  for (i = pAG->tamanhoPopulacao-1; i > pAG->tamanhoPopulacao-pAG->quantidadeLeituraMemoriaATEAMS; i--) {
    switch (pAG->politicaLeitura) {
    case 1: /* escolhe a melhor solucao */
      aux = retornaElemento (principal, l, msize);
      l++;
      break;
    case 2: /* escolhe a sequencias randomicamente[Probabilidade Uniforme] */
      j = (rand() % msize) + 1;
      aux = retornaElemento (principal, j, msize);
      break;
    case 3: /* Lê np sequencias com Probabilidade Linear da melhor para a pior */
      j = (1 + msize) - sequenciaProbLinear (msize, vetprob);
      aux = retornaElemento (principal, j, msize);
      break;
    case 4: /* Lê np sequencias com Probabilidade Linear da pior para a melhor */
      j = sequenciaProbLinear (msize, vetprob);
      aux = retornaElemento (principal, j, msize);
      break;
    case 5: /* Lê np sequencias randomicamente[Probabilidade Uniforme] da pior para melhor */
      j = (rand() % msize) + 1;
      aux = retornaElemento (principal, msize-j, msize);
      break;
    default:
      printf("Politica de leitura nao definida: %d.\n", pAG->politicaLeitura);
      break;
    }
    for (j = 0; j < dados->M; j++) {
      for (k = 0; k < dados->N; k++) {
        (*pop)[i][j][k] = aux->sequence[j][k];
      }
    }
    vmkp[i].mkp = aux->makespan;
    vmkp[i].ok = 1;
  }
}



int *vetProbAG (int tamanhoVetor, int porcentagem, int *vetprob)
{
  int i;
  int iteracoes = tamanhoVetor * porcentagem;

  if (iteracoes == 0)
    iteracoes = 1;

  for (i = 0; i < tamanhoVetor; i++)
    vetprob[i] = 0;
  for (i = 0; i < iteracoes; i++) {
    vetprob[rand()%tamanhoVetor] = 1;
  }
  vetprob[0] = 0;
  return vetprob;
}



void selecaoAleatoria (int tamanho, int *pai1, int *pai2)
{
  *pai1 = rand() % (tamanho-1) + 1; /* Nunca pega a melhor solucao */
  do {
    *pai2 = rand() % (tamanho-1) + 1; /* Nunca pega a melhor solucao */
  } while (*pai1 == *pai2);
}

void quebra2pontos (int tamanho, int *q1, int *q2)
{
  int tmp;
  *q1 = rand() % (tamanho-1) + 1;
  do {
    *q2 = rand() % (tamanho-1) + 1;
  } while (*q1 == *q2);
  if (*q1 > *q2) {
    tmp = *q1;
    *q1 = *q2;
    *q2 = tmp;
  }
}

void crossoverOX (Dados *d, int **pai1, int **pai2, int *mksp, int **filho)
{
  int q1, q2;
  int i, j, j1, j2;
  int **f1visitado, **f2visitado;
  int **filho1, **filho2;
  int ***esc1, ***esc2, ***esc;
  int mksp1, mksp2;

  f1visitado = (int**) malloc ((d->M)*sizeof(int*));
  f2visitado = (int**) malloc ((d->M)*sizeof(int*));
  for (i = 0; i < d->M; i++) {
    f1visitado[i] = (int*) malloc ((d->N)*sizeof(int));
    f2visitado[i] = (int*) malloc ((d->N)*sizeof(int));
    for (j = 0; j < d->N; j++) {
      f1visitado[i][j] = 0;
      f2visitado[i][j] = 0;
    }
  }

  filho1 = alocaMatriz (d->M, d->N);
  filho2 = alocaMatriz (d->M, d->N);

  quebra2pontos (d->N, &q1, &q2);

  /* Copia trecho entre q1 e q2 dos pais para os filhos */
  for (i = 0; i < d->M; i++) {
    for (j = q1; j < q2; j++) {
      filho1[i][j] = pai1[i][j];
      filho2[i][j] = pai2[i][j];
      f1visitado[i][pai1[i][j]] = 1;
      f2visitado[i][pai2[i][j]] = 1;
    }
  }

  /* Faz uma busca no pai do outro filho, por jobs que não estão na subcadeia herdada */
  for (i = 0; i < d->M; i++) {
    j1 = j2 = 0;
    for (j = 0; j < d->N; j++) {
      if (!f1visitado[i][pai2[i][j]]) {
        filho1[i][j1] = pai2[i][j];
        j1++;
        if (j1 == q1)
          j1 = q2;
      }
      if (!f2visitado[i][pai1[i][j]]) {
        filho2[i][j2] = pai1[i][j];
        j2++;
        if (j2 == q1)
          j2 = q2;
      }
    }
  }

  mksp1 = makespan (d->m_op, d->t_op, filho1, d->N, d->M, &esc1);
  mksp2 = makespan (d->m_op, d->t_op, filho2, d->N, d->M, &esc2);

  if (mksp1 < mksp2 && mksp1 != 0) {
    copiaMatriz (d->M, d->N, filho1, filho);
    *mksp = mksp1;
  }
  else if (mksp1 > mksp2 && mksp2 != 0) {
    copiaMatriz (d->M, d->N, filho2, filho);
    *mksp = mksp2;
  }
  else {
    copiaMatriz (d->M, d->N, pai1, filho);
    *mksp = makespan (d->m_op, d->t_op, filho, d->N, d->M, &esc);
    liberaVetorDeMatrizes (d->M, d->N, esc);
  }

  if (mksp1)
    liberaVetorDeMatrizes (d->M, d->N, esc1);
  if (mksp2)
    liberaVetorDeMatrizes (d->M, d->N, esc2);
  liberaMatriz (d->M, filho1);
  liberaMatriz (d->M, filho2);
  liberaMatriz(d->M, f1visitado);
  liberaMatriz(d->M, f2visitado);
}

void mutacao (Dados *d, int **filho, int *mksp)
{
  int q1, q2;
  int i, j, k;
  int **filho_m;
  int ***esc;
  int mksp_m;

  if (filho == NULL) {
    printf("Erro: filho == NULL, em mutacao().\n");
    exit(1);
  }
  filho_m = alocaMatriz (d->M, d->N);

  quebra2pontos (d->N, &q1, &q2);
  //printf("%d %d\n", q1, q2);

  /* Copia inverte o trecho entre q1 e q2 do pai para o filho */
  for (i = 0; i < d->M; i++) {
    for (j = 0; j < q1; j++) {		/* copia trecho sem alteracao */
      filho_m[i][j] = filho[i][j];
    }
    for (j = q1, k = 1; j < q2; j++, k++) {		/* copia trecho invertido */
      filho_m[i][j] = filho[i][q2-k];
    }
    for (j = q2; j < d->N; j++) {	/* copia trecho sem alteração */
      filho_m[i][j] = filho[i][j];
    }
  }

  //mostraMatriz (d->M, d->N, filho);
  //mostraMatriz (d->M, d->N, filho_m);

  mksp_m = makespan (d->m_op, d->t_op, filho_m, d->N, d->M, &esc);
  if (mksp_m > 0) {
    *mksp = mksp_m;
    liberaVetorDeMatrizes (d->M, d->N, esc);
    copiaMatriz(d->M, d->N, filho_m, filho);
  }
  else {
    *mksp = makespan (d->m_op, d->t_op, filho, d->N, d->M, &esc);
    liberaVetorDeMatrizes (d->M, d->N, esc);
  }
  liberaMatriz (d->M, filho_m);
}

void genetico (ParametrosATEAMS *pATEAMS, ParametrosAG *pAG, Dados *dados, int *msize, no **principal, int *vetprob, int ****memoriaAG)
{
  int i, j;
  int ***esc;
  mkord *vaux;
  int *vetprobMutacoes;
  int *vetprobCrossover;
  int ***novaMemoriaAG;
  int ***memoriaTmp;
  int ***tmp;
  int p1, p2;
  Mkp *vmkp;
  int **filho;

  //  printf("Makespan AG: ");

  vetprobMutacoes = (int*) malloc (pAG->tamanhoPopulacao * sizeof(int));
  vetprobCrossover = (int*) malloc (pAG->tamanhoPopulacao * sizeof(int));

  memoriaTmp = (int ***) malloc(pAG->tamanhoPopulacao * sizeof(int **));
  novaMemoriaAG = (int ***) malloc(pAG->tamanhoPopulacao * sizeof(int **));
  vmkp = (Mkp *) malloc(pAG->tamanhoPopulacao * sizeof(Mkp));
  for (i = 0; i < pAG->tamanhoPopulacao; i++) {
    novaMemoriaAG[i] = (int **) malloc(dados->M * sizeof(int *));
    for (j = 0; j < dados->M; j++)
      novaMemoriaAG[i][j] = (int *) malloc(dados->N * sizeof(int));
    vmkp[i].mkp = 0;
    vmkp[i].ok = 0;
  }
  vaux = (mkord *) malloc(pAG->tamanhoPopulacao * sizeof(mkord)); /* usado na ordenação da população */

  leituraMemoriaATEAMS (dados, pATEAMS, pAG, *msize, *principal, memoriaAG, vmkp, vetprob);
  ordenaPopulacao (vaux, pAG->tamanhoPopulacao, vmkp, *memoriaAG, memoriaTmp);
  tmp = *memoriaAG;
  *memoriaAG = memoriaTmp;
  memoriaTmp = tmp;

  int p = 0;
  do {
    /* nova_população <- {}; */
    vetProbAG (pAG->tamanhoPopulacao, pAG->probabilidadeMutacoes, vetprobMutacoes);
    vetProbAG (pAG->tamanhoPopulacao, pAG->probabilidadeCrossover, vetprobCrossover);
    for (i = 0; i < pAG->tamanhoPopulacao; i++) {//for (i = 0; i < pAG->tamanhoPopulacao; i++) {
      filho = novaMemoriaAG[i];
      if (vetprobCrossover[i]) {
        selecaoAleatoria (pAG->tamanhoPopulacao, &p1, &p2);
        //mostraMatriz (dados->M, dados->N, filho);
        crossoverOX (dados, (*memoriaAG)[p1], (*memoriaAG)[p2], &(vmkp[i].mkp), filho);
        vmkp[i].ok = 1;
        //mostraMatriz (dados->M, dados->N, filho);
      }
      else {
        copiaMatriz (dados->M, dados->N, (*memoriaAG)[i], filho);
        vmkp[i].ok = 1;
        vmkp[i].mkp = makespan (dados->m_op, dados->t_op, filho, dados->N, dados->M, &esc);
        liberaVetorDeMatrizes (dados->M, dados->N, esc);
      }
      if (vetprobMutacoes[i]) {
        mutacao (dados, filho, &(vmkp[i].mkp));
        vmkp[i].ok = 1;
      }

    }
    /*for (i = 0; i < pAG->tamanhoPopulacao; i++) {
      mostraMatriz(dados->M, dados->N, novaMemoriaAG[i]);
      }*/
    ordenaPopulacao (vaux, pAG->tamanhoPopulacao, vmkp, novaMemoriaAG, memoriaTmp);
    tmp = *memoriaAG;
    *memoriaAG = memoriaTmp;
    memoriaTmp = novaMemoriaAG;
    novaMemoriaAG = tmp;
    p++;
    if (pATEAMS->makespanBest == vmkp[0].mkp) {
      printf("AG parada %d.\n", p);
      break;
    }
  } while (pAG->numeroIteracoes > p);

  escreveGenetico (dados, pATEAMS, msize, vetprob, principal, (*memoriaAG)[0], vmkp);

  liberaVetorDeMatrizes (pAG->tamanhoPopulacao, dados->M, novaMemoriaAG);

  free (memoriaTmp);
  free (vetprobMutacoes);
  free (vetprobCrossover);
  free (vmkp);
  free (vaux);
}
