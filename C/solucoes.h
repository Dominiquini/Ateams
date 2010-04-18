/*
 * solucoes.h
 *
 *  Created on: 07/03/2010
 *      Author: iha
 */

#ifndef SOLUCOES_H_
#define SOLUCOES_H_
#include "io.h"
#include "lista.h"

int geraSolucaoAleatoria (int **solucao, Dados *dados);

int **novaSolucaoTrocaAB (int **sequencia, int A, int B, int linha, int coluna, int m);

void inicializaMemoriaATEAMS (int *tamanhoMemoriaATEAMS, no **lista);

void populaMemoriaATEAMS (Dados *dados, ParametrosATEAMS *pATEAMS, int *tamanhoMemoriaATEAMS, no **lista);


#endif /* SOLUCOES_H_ */
