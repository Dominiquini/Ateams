/*
 * lista.c
 *
 *  Created on: 07/03/2010
 *      Author: iha
 */

#include "lista.h"
#include <stdlib.h>
#include <stdio.h>

no *novaLista (void)
{
  return NULL;
}

/* Cria um no' com uma solucao, e seu makespan
 * int **seq : uma solucao no formato de matriz Tarefas por Maquinas
 * int makespan : makespan da solucao
 * */
no *novoNo (int **seq, int makespan) {
  no *apno;

  apno = (no *) malloc(sizeof(no));
  apno->sequence = seq;
  apno->makespan = makespan;
  apno->bt = 0;
  apno->prox = NULL;

  return apno;
}

int listaVazia(no *lista) {
  return(lista == NULL);
}

void liberaNo (no *noLista, int n) {
  int i;

  for (i = 0; i < n; i++)
    free(noLista->sequence[i]);
  free(noLista->sequence);
  free(noLista);
  noLista = NULL;
}

void liberaLista (no *lista, int n) {
  no *p;
  p = lista;

  while (p != NULL) {
    lista = p->prox;
    liberaNo(p, n);
    p = lista;
  }
  lista = NULL;
}

void imprimeLista (no *lista, int N, int M) {
  no *aux;
  int i, j;

  aux = lista;

  while (aux != NULL) {
    for(i = 0; i< M; i++) {
      for(j = 0; j < N; j++)
        printf("%d ",aux->sequence[j][i]);
    }
    printf("%5d  ",aux->makespan);
    printf("%5d  ",aux->bt);
    aux = aux->prox;
    printf("\n");
  }
}

void insereListaInic (no **lista, no *noLista, int *msize) {
  noLista->prox = *lista;
  *lista = noLista;
  (*msize)++;
}

void removeListaInic (no **lista, int M, int *msize) {
  no *p = *lista;

  *lista=(*lista)->prox;
  liberaNo(p, M);
  (*msize)--;
}

void insereLista (no **lista, no *noLista, int *msize)
{
  no *aux, *aux2;

  aux = *lista;
  aux2 = NULL;

  while((aux!=NULL) && (aux->makespan < noLista->makespan))
    {
      aux2 = aux;
      aux = aux->prox;
    }

  if(aux2 == NULL)
    insereListaInic(lista, noLista, msize);
  else
    insereListaInic((no **)&(aux2->prox), noLista, msize);
}


int removeLista (no **lista, int n, int M, int *msize) {
  no *aux;
  int i = 1;

  if (!listaVazia(*lista)) {
    aux= *lista;

    if (*msize < n)
      return 0;
    else {
      while (i < (n-1)) {
        aux = aux->prox;
        i++;
      }
      if (n > 1)
        removeListaInic((no **)&(aux->prox), M, msize);
      else
        removeListaInic(lista, M, msize);
      return 1;
    }
  }
  return 0;
}

no *retornaElemento (no *lista, int n, int msize) {
  no *aux;
  int i;

  aux = lista;
  if (msize < n)
    return NULL;
  else {
    for (i = 1; i < n; i++)
      aux = aux->prox;
    return aux;
  }
}




node *newnode (int **input)
{
  node *new;
  new = (node *) malloc(sizeof(node));
  new->sequence = input;
  new->next = NULL;
  return new;
}

mknode *newmknode (int mkspan)
{
  mknode *mknew;
  mknew = (mknode *) malloc( sizeof( mknode ) );
  mknew->span = mkspan;
  mknew->list = NULL;
  mknew->prox = NULL;

  return mknew;
}

/* Insere elemento no fim da lista de valores visitados */
void insert (int **elemento, int mkspan, mknode **root, int N)
{
  mknode *seek,*aux;
  node *end;
  seek = *root;
  if (*root == NULL) {/* 1o. elemento da lista (makespan e seq) */
    *root = newmknode(mkspan);
    seek = *root;
    seek->list = newnode(elemento);
  }
  else {
    aux = seekspan(mkspan, root);
    if (aux != NULL) {
      end = aux->list;
      while (end->next != NULL)
        end = end->next;
      end->next = newnode(elemento);
    }
    else {
      while (seek->prox != NULL)
        seek = seek->prox;
      seek->prox = newmknode(mkspan);
      seek = seek->prox;
      seek->list= newnode(elemento);
    }
  }
}

mknode *seekspan (int mkspan, mknode **root)
{
  mknode *seek;

  seek = *root;
  while ((seek != NULL) && (seek->span != mkspan))
    seek = seek->prox;

  return seek;
}


/* seekseq()
 * Funcao:	Verifica se a solucao "elemento" esta na memoria.
 * Entrada:	int **elemento matriz com uma solucao;
 *              node **lista memoria de solucoes.
 * Saida:	1 se verdade;
 *              0 se falso.
 * */
int seekseq (int **elemento, node *lista, int N, int M)
{
  int i, j;
  int present;

  while (lista != NULL) {
    present = 1;
    for (i = 0; present && i < N; i++) {
      for (j = 0; present && j < M; j++) {
        if (lista->sequence[i][j] == elemento[i][j])
          present = 1;
        else
          present = 0;
      }
      if (present)
        return present;
      lista = lista->next;
    }
  }
  return 0;
}

/* Search()
 * Funcao:    Verifica se o "elemento" esta na lista de solucoes percorridas.
 * Entrada:   int **elemento matriz com uma solucao;
 *            mknode **root lista de solusoes exploradas;
 *            int N: numero de tarefas;
 *            int mkspan: makespan da solucao "elemento".
 * Saida:     Se o elemento estiver na lista devolve 1, caso contrario 0.
 *
 * */
int Search(int **elemento, mknode **root, int N, int M, int mkspan)
{
  mknode *mseek, *aux;
  node *seek;

  mseek = *root;
  aux = seekspan(mkspan, root);
  if (aux == NULL)
    return 0;
  else {
    seek = aux->list;

    return (seekseq(elemento, seek, N, M));
  }
}


long showlist (mknode **root, int N, FILE *resultado)
{
  mknode *mseek;
  node *seek;
  long somall = 0;
  long somap;
  long totalm = 0;
  mseek = *root;

  while (mseek != NULL)
    {
      totalm++;
      fprintf(resultado,"\n%3d ->",mseek->span);
      seek = mseek->list;
      somap = 0;
      while (seek != NULL)
        {
          seek = seek->next;
          somap++;
        }
      mseek= mseek->prox;
      fprintf(resultado," %ld",somap);
      somall = somall + somap;
    }
  fprintf(resultado,"\n total de makespans na lista: %ld", totalm);
  fprintf(resultado,"\n total de elementos na lista: %ld", somall);
  return(somall);
}

void apagalista (mknode **root, int N, int M)
{
  mknode *mseek;
  node *seek;
  int i;
  mseek = *root;

  while (*root != NULL) {
    mseek = *root;
    *root = mseek->prox;
    while (mseek->list != NULL)	{
      seek = mseek->list;
      mseek->list = seek->next;
      for (i = 0; i < N; i++)
        free(seek->sequence[i]);
      free(seek->sequence);
      seek->next=NULL;
      seek->sequence=NULL;
      free(seek);
      seek=NULL;
    }
    mseek->prox=NULL;
    free(mseek);
  }
}
