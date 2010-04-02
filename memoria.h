/*
 * memoria.h
 *
 *  Created on: 06/03/2010
 *      Author: iha
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

#include "defs.h"
#include "lista.h"
#include "io.h"

int *vetorProb(int msize);
void liberaVetorProb (int *vetprob);

int sequenciaProbLinear (int msize, int *vetprob);

void politicaDestruicao1 (int n, int N, int *msize, no **lista);
void politicaDestruicao2 (int N, int *msize, no **lista);
void destruidor (int M, int pd, int *msize, int *vetprob, no **lista);
void finalDestruidor (int *vetprob);

void politicaAceitacao1 (int **seq, int makespan, int M, int pd, int *msize, int *vetprob, no **lista);
void politicaAceitacao2 (int **seq, int makespan, int M, int pd, int *msize, int *vetprob, no **lista);
void politicaAceitacao (int **seq, int makespan, Dados *dados, ParametrosATEAMS *pATEAMS, int *msize, int *vetprob, no **lista);


void copiaMatriz (int linhas, int colunas, int **A, int **B);

int **alocaMatriz (int linhas, int colunas);


void liberaMatriz (int linhas, int **M);
void liberaVetorDeMatrizes (int a, int b, int ***vetor);

#endif /* MEMORIA_H_ */
