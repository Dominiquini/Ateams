/*
 * memoria.h
 *
 *  Created on: 06/03/2010
 *      Author: iha
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_


void copiaMatriz (int linhas, int colunas, int **A, int **B);

int **alocaMatriz (int linhas, int colunas);


void liberaMatriz (int linhas, int **M);
void liberaVetorDeMatrizes (int a, int b, int ***vetor);

#endif /* MEMORIA_H_ */
