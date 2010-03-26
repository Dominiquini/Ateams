/*
 * memoria.c
 *
 *  Created on: 06/03/2010
 *      Author: iha
 *      Descricao: cuida da alocação e desalocação de memória do programa.
 */
#include <stdlib.h>
#include "memoria.h"



int **alocaMatriz (int linhas, int colunas)
{
	int i;
	int **M;

	M = (int**) malloc (linhas * sizeof (int*));
	for (i = 0; i < linhas; i++)
		M[i] = (int*) malloc (colunas * sizeof (int));
	return M;
}

void liberaMatriz (int linhas, int **M)
{
	int i;

	for (i = 0; i < linhas; i++) {
		if (M[i])
			free (M[i]);
	}
	if (M)
		free (M);
}

void liberaVetorDeMatrizes (int a, int b, int ***vetor) {
	int i, j;
	for (i = 0; i < a; i++) {
		for (j = 0; j < b; j++) {
			if (vetor[i][j])
				free(vetor[i][j]);
		}
		if (vetor[i])
			free (vetor[i]);
	}
	if (vetor)
		free (vetor);
}



/* Libera a memória restante */
void liberaMain ()
{

}
