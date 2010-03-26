/*
 * politicasMemoria.c
 *
 *  Created on: 07/03/2010
 *      Author: iha
 */

#include "politicasMemoria.h"
#include "defs.h"
#include "lista.h"
#include <stdlib.h>
#include <stdio.h>

int *vetorProb(int msize) {
	int i;
  	int *vetprob;
	vetprob = (int*) malloc(msize * sizeof(int));
	vetprob[0] = 0;
	/*printf(" VETPROB "); */
	for(i = 1; i < msize; i++) {
		vetprob[i] = vetprob[i-1] + i;
/*    printf(" %d ",vetprob[i]);*/
	}
	return vetprob;
}

/*******************************************************************************
	Retorna uma das sequencias de acordo com a Probabilidade Linear de escolha
	Melhor sequencia: probabilidade 0
********************************************************************************/
int sequenciaProbLinear(int msize, int *vetprob) {
	int k = 0;
	int inter = 0;

	if (vetprob == NULL) {
		printf("\n Erro em sequenciaProbLinear: vetprob = NULL\n");
		exit(1);
	}

	do {
    	inter = rand() % vetprob[msize-1];
    	while (vetprob[k] <= inter) k++;
  	} while (k == 0);
  	k++;
	return k;
}

/* Destroi pior solucao */
void politicaDestruicao1 (int n, int M, int *msize, no **lista)
{
	removeLista (lista, n, M, msize);
}


/* Destroi segundo uma probabilidade Uniforme a menos da melhor solucao */
void politicaDestruicao2(int N, int *msize, no **lista) {
  	int k;

  	do {
    	k = (rand() % *msize) + 1;
  	} while (k == 1);
  	 /*printf("K= %d ",k);*/
  	removeLista(lista, k, N, msize);
}

/* Destroi segundo uma probabilidade Linear a menos da melhor solucao */
void politicaDestruicao3(int N, int *msize, int *vetprob, no **lista) {
	int k = 0;

	k = sequenciaProbLinear(*msize, vetprob);
	/*printf("K= %d ",k);*/
	removeLista(lista, k, N, msize);
}


void destruidor (int M, int pd, int *msize, int *vetprob, no **lista) {
	switch (pd) {
		case 1:
			/* Destroi pior solucao */
			politicaDestruicao1(*msize, M, msize, lista);
      		break;
    	case 2:
    		/* Destroi segundo uma probabilidade Uniforme a menos da melhor solucao */
      		politicaDestruicao2(M, msize, lista);
      		break;
    	case 3:
    		/* Destroi segundo uma probabilidade Linear a menos da melhor solucao */
      		politicaDestruicao3(M, msize, vetprob, lista);
      		break;
    	default:
      		printf("Erro PD");
      		break;
	}
}

void liberaVetorProb (int *vetprob)
{
	free(vetprob);
	vetprob = NULL;
}


/* Aceita qualquer solucao */
void politicaAceitacao1 (int **seq, int makespan, int M, int pd, int *msize, int *vetprob, no **lista)
{
	no *nolista;
	destruidor (M, pd, msize, vetprob, lista);
	nolista = novoNo (seq, makespan);
	insereLista (lista, nolista, msize);
}

/* Aceita apenas se for melhor que a pior solucao */
void politicaAceitacao2 (int **seq,int makespan, int M, int pd, int *msize, int *vetprob, no **lista)
{
  no *aux;
  no *nolista;
  /* printf("msize=%d",msize); */
  aux=retornaElemento(*lista, *msize, *msize);
  if((aux->makespan)>makespan){
    /* printf("MKP: %d       ",aux->makespan); */
    /* for(i=0;i<N;i++) printf("%d ",aux->sequence[i]);*/
    destruidor(M, pd, msize, vetprob, lista);
    nolista = novoNo (seq, makespan);
    insereLista(lista,nolista, msize);
  }
  else
    free(seq);
}


/* Na politica de aceitação verifico se a solução já existe */
void politicaAceitacao (int **seq, int makespan, Dados *dados, ParametrosATEAMS *pATEAMS, int *msize, int *vetprob, no **lista) {
	switch (pATEAMS->politicaAceitacao) {
		case 1:
			/* Aceita qualquer solucao */
			politicaAceitacao1 (seq, makespan, dados->M, pATEAMS->politicaDestruicao, msize, vetprob, lista);
			break;
		case 2:
			/* Aceita apenas se for melhor que a pior solucao */
			politicaAceitacao2 (seq, makespan, dados->M, pATEAMS->politicaDestruicao, msize, vetprob, lista);
      		break;
    	default:
      		printf("Erro PA do Destruidor");
      	break;
    }
}
