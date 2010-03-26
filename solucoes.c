/*
 * solucoes.c
 *
 *  Created on: 07/03/2010
 *      Author: iha
 *      Descricao: Funções para manipular/gerar soluções.
 */
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/types.h>

#include "lista.h"
#include "defs.h"
#include "io.h"
#include "makespan.h"

int *mixvet (int *vet, int tamanho);
int geraSolucaoAleatoria (int **solucao, Dados *dados);

/* Dada uma solucao, retorna uma nova solucao, com as posicoes A e B trocadas */
int **novaSolucaoTrocaAB (int **sequencia, int A, int B, int linha, int coluna, int m) {
	int **change;
	int aux, j, i;

	change = (int **) malloc (linha * sizeof(int*));
	for (i = 0; i < linha; i++)
		change[i] = (int *) malloc(coluna * sizeof(int));

	for (i = 0; i < linha; i++)
		for (j = 0; j < coluna; j++)
			change[i][j] = sequencia[i][j];

	aux = change[m][A];
	change[m][A] = change[m][B];
	change[m][B] = aux;

	return change;
}

void inicializaMemoriaATEAMS (int *tamanhoMemoriaATEAMS, no **lista)
{
	*tamanhoMemoriaATEAMS = 0;
	*lista = novaLista ();
}

void populaMemoriaATEAMS (Dados *dados, ParametrosATEAMS *pATEAMS, int *tamanhoMemoriaATEAMS, no **lista)
{
	int i, j;
  	int ***popaux;	/* vetor de solucoes MxN */
  	no *nolista;
  	int mksp = 0;
	/* Cria um vetor de matrizes MxN, conterá a sequencia de jobs em cada máquina */
  	popaux = (int ***) malloc(pATEAMS->tamanhoPopulacao * sizeof(int **));
  	for (i = 0; i < pATEAMS->tamanhoPopulacao; i++) {
    	popaux[i] = (int **) malloc(dados->M * sizeof(int*));
    	for (j = 0; j < dados->M; j++)
	  		popaux[i][j] = (int *) malloc(dados->N * sizeof(int));
   		mksp = geraSolucaoAleatoria(popaux[i], dados);
		nolista = novoNo(popaux[i], mksp);
		insereLista(lista, nolista, tamanhoMemoriaATEAMS);
	}
  	free (popaux);
}



int geraSolucaoAleatoria (int **solucao, Dados *dados) {
    int i,j, k;
    int mksp;
    int *vet, *maq;
    int ***esc;

    vet = (int *) malloc(dados->N * sizeof(int));
    maq = (int *) malloc(dados->M * sizeof(int)); /* indica próxima operacao da máquina */

	for (i = 0; i < dados->M; i++)
		maq[i] = 0;
	for (k = 0; k < dados->N; k++)
		vet[k] = k;
	for (i = 0; i < dados->M; i++) {
		vet = mixvet(vet, dados->N);
		for (j = 0; j < dados->N; j++) {
			solucao[dados->m_op[vet[j]][i]][maq[dados->m_op[vet[j]][i]]] = vet[j];
			maq[dados->m_op[vet[j]][i]] += 1;
		}
	}
	mksp = makespan (dados->m_op, dados->t_op, solucao, dados->N, dados->M, &esc);
    superFree (3, esc, dados->M, dados->N);
    free (vet);
    free (maq);
    return mksp;
}

int *mixvet (int *vet, int tamanho) {
    int i;
    int aux;
    int *vaux;
    int c1, c2;
    int change;
    struct timeb rd;

    vaux = vet;
    ftime(&rd);
    aux = rd.millitm;

    for (i = 0; i < aux ; i++) {
       	c1 = rand() % tamanho;
       	do {
       		c2 = rand() % tamanho;
       	} while (c2 == c1);
       	change = vaux[c1];
       	vaux[c1] = vaux[c2];
       	vaux[c2] = change;
    }
	return (vaux);
}
