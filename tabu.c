/*
 * tabu.c
 *
 *  Created on: 06/03/2010
 *      Author: iha
 */

#include "io.h"
#include "tabu.h"
#include "lista.h"
#include "memoria.h"
#include "makespan.h"
#include "solucoes.h"
#include "politicasMemoria.h"
#include <stdlib.h>
#include <stdio.h>


int **leTabu1 (int *makespan, ParametrosBT *pBT, Dados *dados, int *msize, int *vetprob, no *lista);
int **leTabu2 (int *makespan, ParametrosBT *pBT, Dados *dados, int *msize, int *vetprob, no *lista);
int **leTabu3 (int *makespan, Dados *dados, int msize, no *lista);
int **leTabu4 (int *makespan, Dados *dados, int msize, no *lista);
int **leTabu (int *makespan, ParametrosBT *pBT, Dados *dados, int *msize, int improved, int *vetprob, no *lista);
void escreveTabu (int **seq, int makespan, int eh_nova, Dados *dados, ParametrosATEAMS *pATEAMS, int *msize, int *improved, int *vetprob, no **lista);
void tabucount (int *Tabu, int posicao, int tamanho);
void inicializaTamanhoListaTabu (TamanhoTabu *t, int iteracoes);
void listaTabuDinamica (int iteracoes, Dados *d, ParametrosBT *pBT);


void tabu (ParametrosATEAMS *pATEAMS, ParametrosBT *pBT, Dados *dados, int *msize, no **lista, int *vetprob)
{
  int **starts;			/* Sequencia inicial */
  int **newval=NULL;		/* Nova sequencia gerada por permuta de elementos */
  int fnewval = 0;
  int **globaltabu;		/* sequencia de maior valor global (todas iterações até entao) */
  int **local;			/* sequencia de maior valor dentro de cada iteracao */
  int flocal = 0, fglobal;	/* valores das funcoes do maximo local e global */
  int posicao = 0;		/* posicao trocada que gerou maximo local */
  int stepcounter;		/* contador de passos */
  int *Tabu;			/* Estrutura que controla a atribuicao de TABU */
  int elmarker = 0;		/* marca elemento a ser trocado com elemento fixo */
  int elm = 0;			/* marca qual posicao da sequencia deve ser ser fixada */
  int m = 0;
  int itcounter = 0;		/* contador de iteracoes  */
  int prescounter= 0;		/* contador de presencas na lista */
  int fiter;			/* armazena iteracao na qual atingiu-se valor de minimo */
  int i,j;
  long ocio;
  int help = 0;
  mknode **root;			/* Raiz lista de sequencias exploradas */
  int **seqaux;
  int eh_nova=0;
  int parada[] = {0, 0, 0};
  int improved = 0;               /* memoria melhorou(1) ou nao (0) */
  int ***esc;
  TamanhoTabu t;

  pBT->tamanhoListaTabu = dados->N;
  globaltabu = leTabu (&fglobal, pBT, dados, msize, improved, vetprob, *lista);	/* busca valor de entrada -- Politica de Leitura */
  /* globaltabu recebe uma matriz com uma solucao */
  /* fglobal e' o makespan da solucao */

  inicializaTamanhoListaTabu (&t, pBT->k);

  if (globaltabu != NULL) {
    root = (mknode **) malloc(sizeof(mknode *));

    starts = globaltabu; /* valor inicial para a execucao das iteracoes */

    /* Inicializacao */
    *root = NULL;
    local = NULL;

    insert (globaltabu, fglobal, root, dados->M);

    /* Inicializa estrutura Tabu */
    Tabu = (int *) malloc((pBT->tamanhoListaTabu + 1) * sizeof(int));
    for (i = 0; i <= pBT->tamanhoListaTabu; i++)
      Tabu[i] = 0;

    /****  Loop de Execucao do Algoritmo  ****/
    itcounter = 0;
    elmarker = 0;
    elm = 0;
    ocio = 0;
    for (;;) {
      Tabu[pBT->tamanhoListaTabu] = 0;            /* Contador de Tabus atribuidos em cada iteracao */
      itcounter++;
      stepcounter = 0;
      prescounter = 0;
      while (stepcounter < pBT->tamanhoListaTabu-1) {    /* Cada iteracao executa N-1 passos */
        /* Vamos pegar uma seq. qualquer, modifica-la e verificar se o resultado obtido jah foi visitado antes. Isso
           eh verificado procurando-se o valor em "root". Se não foi visitado e eh melhor que o valor local atual,
           passa a ser o valor atual e é marcado na lista tabu.*/

        stepcounter++;
        if (elmarker == elm) {
          if (elmarker == (dados->N-1))
            elmarker = 0;
          else
            elmarker++;
        }
        if (Tabu[elmarker] == 0) {     /* Verifica Tabu */
          m = rand () % dados->M;
          newval = novaSolucaoTrocaAB(starts, elm, elmarker, dados->M, dados->N, m); /* calcula nova sequencia */
          fnewval = makespan(dados->m_op, dados->t_op, newval, dados->N, dados->M, &esc);
          if (fnewval > 0)
            superFree(3, esc, dados->M, dados->N);
          if (fnewval != 0 && (Search(newval, root, dados->M, dados->N, fnewval) || flocal > fnewval)) {  /* Verifica se newval foi visitado */
            local = newval;
            posicao = elmarker;
            flocal = fnewval;
            insert(newval, fnewval, root, dados->M);
          }
          else {
            prescounter++;
            for (i = 0; i < dados->M; i++)
              free(newval[i]);
            free(newval);
            newval = NULL;
          }
        }
        else /*** Tabu ***/
          Tabu[pBT->tamanhoListaTabu]++;
        elmarker++;
        if (elmarker > (dados->N-1))
          elmarker = 0;
      } /* Fim dos passos - Aqui fecha-se uma iteracao TABU */
      elm++;
      if (elm > dados->N-1)    /* Acerta posicao fixa */
        elm = 0;
      if (prescounter < (pBT->tamanhoListaTabu-1))
        Tabu[posicao] = 1;       /* Nova atribuicao devido ao fim da iteracao   */

      /*** Controle do periodo de vigencia da atribuicao Tabu */
      tabucount(Tabu, posicao, pBT->tamanhoListaTabu);

      if (local == NULL) {
        for (i = 0; i < pBT->tamanhoListaTabu-1; i++)    /* Repete starts anterior com relaxamento de Tabu */
          Tabu[i] = 0;
      }
      else {
        starts = local;          /* max da iteracao anterior = novo start */
        if (flocal <= fglobal) {/* atualiza max global */ /*Adicionado "=" para modificar sequencia sempre que possivel*/
          globaltabu = local;
          fglobal = flocal;
          fiter = itcounter;
          help = 1;
          eh_nova=1;
        }
      }
      if (help == 1) {/* atualizacao da variavel ocio */
        help = 0;
        ocio = 0;
      }
      else
        ocio++;

      if (pATEAMS->makespanBest == flocal) {
        printf("BT parada %d.\n", itcounter );
        break;
      }

      local = NULL; /* inicializa valor local com valor fora do intervalo */
      /*condicoes de parada */
      if (pBT->numeroIteracoes > 0)
        if (itcounter > pBT->numeroIteracoes) {
          parada[0] = 1;
          /*printf("Contagem %d.\n", itcounter);*/
          break;
        }
      /*
        if (porcentagem > 0 && (porcentagem*(dados->N-1)/100) <= (Tabu[dados->N] + prescounter)) {
        parada[1] = 1;
        printf("Porcentagem = %d.\n", porcentagem*(dados->N-1)/100);
        break;
        }*/
      /*if (fator > 0)
        if ((fator*dados->N) <= ocio) {
        parada[2]=1;
        printf("Fator\n");
        break;
        }*/
      //listaTabuDinamica (itcounter, dados, pBT, &t);
    } /* Fim das iteracoes */

    seqaux = (int **) malloc(dados->M * sizeof(int*));
    for (i = 0; i < dados->M; i++)
      seqaux[i] = (int *) malloc(dados->N * sizeof(int));
    for (i = 0; i < dados->M; i++) {
      for (j = 0; j < dados->N; j++)
        seqaux[i][j] = globaltabu[i][j];
    }

    escreveTabu (seqaux, fglobal, eh_nova, dados, pATEAMS, msize, &improved, vetprob, lista);
    free(Tabu);
    apagalista(root, dados->M, dados->N);
    free(root);
  }
}

void inicializaTamanhoListaTabu (TamanhoTabu *t, int iteracoes)
{

}



void listaTabuDinamica (int iteracoes, Dados *d, ParametrosBT *pBT)
{
  if (pBT->makespan.atual < pBT->makespan.anterior) {
    if (pBT->tamanhoListaTabu > pBT->tamanho.min) {
      pBT->tamanhoListaTabu -= 1;
    }
    if (pBT->makespan.atual < pBT->makespan.melhor) {
      pBT->makespan.iteracoesSemMelhoria = 0;
    }
  }
  else {
    if (pBT->tamanhoListaTabu < pBT->tamanho.max) {
      pBT->tamanhoListaTabu += 1;
    }
    pBT->makespan.iteracoesSemMelhoria += 1;
    if (pBT->makespan.iteracoesSemMelhoria > pBT->makespan.maxSemMelhoria) {
      /**/
    }
  }
}


void BT (ParametrosATEAMS *pATEAMS, ParametrosBT *pBT, Dados *dados, int *msize, no **lista, int *vetprob)
{
  /* seja S0 a solucao inicial; */
  /* s* <-- s; {Melhor solução obtida até então} */
  /* Iter <-- 0; {Contador do número de itercações} */
  /* MelhorIter <-- 0; {Iteração mais recente que forneceu s*} */
  /* Seja BTmax o número  máximo de iterações sem melhora em s* */
  /* T <-- {} {Lista Tabu} */
  /* Inicialize a função de aspiração A;*/
  /* Enquanto (Iter - MelhorIter <= Btmax) faça */
  /* Iter <-- Iter + 1; */
  /* Seja s' <-- s */

}



int **leTabu (int *makespan, ParametrosBT *pBT, Dados *dados, int *msize, int improved, int *vetprob, no *lista)
{
  int plAnterior = pBT->politicaLeitura;

  if (improved)
    pBT->politicaLeitura = 3;

  switch (pBT->politicaLeitura) {
  case 1:
    return leTabu1 (makespan, pBT, dados, msize, vetprob, lista);
  case 2:
    return leTabu2 (makespan, pBT, dados, msize, vetprob, lista);
  case 3:
    pBT->politicaLeitura = plAnterior;
    return leTabu3 (makespan, dados, *msize, lista);
  case 4:
    pBT->politicaLeitura = 1;
    return leTabu4 (makespan, dados, *msize, lista);
  default:
    printf ("Erro: Politica de leitura Tabu deve estar entre 1-4.\n");
    exit(1);
    break;
  }
  return NULL;
}


/*
 * Lê a da melhor  para a pior com distribuicao linear de probabilidade
 * nunca le uma sequencia ja lida
 */
int **leTabu1 (int *makespan, ParametrosBT *pBT, Dados *dados, int *msize, int *vetprob, no *lista)
{
  no *aux;
  int **seq;
  int i, j, k, n = 0;

  do {
    k = (1+*msize) - sequenciaProbLinear (*msize, vetprob);
    aux = retornaElemento(lista, k, *msize);
    n++;
  } while((aux->bt) && (n < dados->N));

  if (aux->bt == 0) {
    seq = (int**) malloc(dados->M * sizeof(int*));
    for (i = 0; i < dados->M; i++)
      seq[i] = (int*) malloc(dados->N * sizeof(int));
    aux->bt = 1;
    for (i = 0; i < dados->M; i++) {
      for (j = 0; j < dados->N; j++)
        seq[i][j] = aux->sequence[i][j];
    }
    (*makespan) = aux->makespan;
    return seq;
  }
  else
    return NULL;
}


/*
 * le a da pior para a melhor com distribuicao linear de probabilidade
 * nunca le uma sequencia ja lida
 */
int **leTabu2 (int *makespan, ParametrosBT *pBT, Dados *dados, int *msize, int *vetprob, no *lista)
{
  no *aux;
  int **seq;
  int i, j, k, n = 0;

  do {
    k = sequenciaProbLinear(*msize, vetprob);
    aux = retornaElemento(lista, k, *msize);
    n++;
  } while ((aux->bt) && (n < dados->M));


  if (aux->bt == 0) {
    seq = (int**) malloc(dados->M * sizeof(int*));
    for (i = 0; i < dados->M; i++)
      seq[i] = (int*) malloc(dados->N * sizeof(int));
    aux->bt = 1;
    for (i = 0; i < dados->M; i++) {
      for (j = 0; j < dados->N; j++)
        seq[i][j] = aux->sequence[i][j];
    }
    (*makespan) = aux->makespan;
    return (seq);
  }
  return NULL;
}


/*
 *  Lê da melhor para a pior escolhendo a melhor que ainda não leu
 */
int **leTabu3 (int *makespan, Dados *dados, int msize, no *lista)
{
  no *aux;
  int **seq;
  int i, j, k = 0;

  do {
    k++;
    aux = retornaElemento(lista, k, msize);
  } while ((aux->bt) && (k != msize));

  if (aux->bt == 0) {
    seq = (int**) malloc(dados->M * sizeof(int*));
    for (i = 0; i < dados->M; i++)
      seq[i] = (int*) malloc(dados->N * sizeof(int));
    aux->bt = 1;
    for (i = 0; i < dados->M; i++) {
      for (j = 0; j < dados->N; j++)
        seq[i][j] = aux->sequence[i][j];
    }
    (*makespan) = aux->makespan;
    return (seq);
  }
  return NULL;
}

/*
 * Da pior para a melhor escolhendo a pior que ainda não leu
 */
int **leTabu4 (int *makespan, Dados *dados, int msize, no *lista)
{
  no *aux;
  int **seq;
  int i,j, k = msize+1;
  do {
    k--;
    aux = retornaElemento(lista, k, msize);
  } while ((aux->bt) && (k != 1));

  if (aux->bt == 0) {
    seq = (int**) malloc(dados->M * sizeof(int*));
    for (i = 0; i < dados->M; i++)
      seq[i] = (int*) malloc(dados->N * sizeof(int));
    aux->bt = 1;
    for (i = 0; i < dados->N; i++) {
      for (j = 0; j < dados->N; j++)
        seq[i][j] = aux->sequence[i][j];
    }
    (*makespan) = aux->makespan;
    return seq;
  }
  return NULL;
}


void escreveTabu (int **seq, int makespan, int eh_nova, Dados *dados, ParametrosATEAMS *pATEAMS, int *msize, int *improved, int *vetprob, no **lista) {
  no *aux;
  int i;

  /* printf("-------TABU---------\n     makespan = %d \n\n",makespan); */

  aux = retornaElemento(*lista, 1, *msize);
  if (makespan < (aux->makespan))
    *improved = 1;
  else
    *improved = 0;
  if (eh_nova) {
    politicaAceitacao (seq, makespan, dados, pATEAMS, msize, vetprob, lista);
    printf("Makespan BT: %d.\n", makespan);
  }
  else {
    for (i = 0; i < dados->M; i++)
      free(seq[i]);
    free(seq);
  }
  /* espera(); */
}

/*
 * Acerta a atribuicao de Tabus.
 */
void tabucount (int *Tabu, int posicao, int tamanho) {
  int i;
  int duracao;

  duracao = tamanho/2;
  if ((tamanho%2)!= 0)
    duracao++;

  for (i = 0; i < tamanho; i++)
    if ((Tabu[i] != 0) && (i != posicao))
      Tabu[i]++;

  for (i = 0; i < tamanho; i++)
    if (Tabu[i] > duracao)
      Tabu[i] = 0;

}
