/*
 * utils.c
 *
 *  Created on: 10/03/2010
 *      Author: iha
 */


void copiaMatriz (int linhas, int colunas, int **A, int **B)
{
	int i, j;

	for (i = 0; i < linhas; i++) {
		for (j = 0; j < colunas; j++) {
			B[i][j] = A[i][j];
		}
	}
}

