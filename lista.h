/*
 * lista.h
 *
 *  Created on: 07/03/2010
 *      Author: iha
 */

#ifndef LISTA_H_
#define LISTA_H_
#include <stdio.h>

typedef struct {
	int **sequence; /* sequencia de solucoes */
	int makespan;
	int bt;   /*diz se a BT ja leu esta sequencia*/
	void *prox;
} no;

/* A struct Mkp determina o makespan do indivíduo e se já foi calculado */
typedef struct {
	int mkp;
	int ok;
} Mkp;

typedef struct {   	/* tipo no para uma lista generica */
	int **sequence;
	void *next;
} node;

typedef struct {	/* tipo no de makespan - separa seqs por valor de makespan */
	int span;
	void *list;
	void *prox;
} mknode;

typedef struct {	/* tipo auxiliar para geracao da nova populacao */
	int valor;
	int posicao;
} mkord;

typedef struct {	/* tipo criado para dizer se o mkp de determinado individuo(mkp) ja foi calculado(ok) */
	int mkp;
	int ok;
} tipo_mkp;



no *novaLista (void);
no *novoNo (int **seq, int makespan);
int listaVazia (no *lista);
void liberaNo (no *noLista, int n);

void liberaLista (no *lista, int n);
void imprimeLista (no *lista, int N, int M);
void insereListaInic (no **lista, no *noLista, int *msize);
void removeListaInic (no **lista, int M, int *msize);
void insereLista (no **lista, no *noLista, int *msize);
int removeLista (no **lista, int n, int M, int *msize);
no *retornaElemento (no *lista, int n, int msize);


node *newnode (int **input);
mknode *newmknode (int mkspan);
void insert (int **elemento, int mkspan, mknode **root, int N);
mknode *seekspan (int mkspan, mknode **root);
int seekseq (int **elemento, node *lista, int N, int M);
int Search (int **elemento, mknode **root, int N, int M, int mkspan);
long showlist (mknode **root, int N, FILE *resultado);
void apagalista (mknode **root, int N, int M);

#endif /* LISTA_H_ */
