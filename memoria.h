/*
 * memoria.h
 *
 *  Created on: 06/03/2010
 *      Author: iha
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

void liberaVetorDeMatrizes (int a, int b, int ***vetor);


int **alocaMatriz (int linhas, int colunas);
void liberaMatriz (int linhas, int **M);

#endif /* MEMORIA_H_ */
