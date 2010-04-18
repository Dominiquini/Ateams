/*
 * genetico.h
 *
 *  Created on: 07/03/2010
 *      Author: iha
 */

#ifndef GENETICO_H_
#define GENETICO_H_

#include "io.h"
#include "lista.h"


void imprimeSequenciaGenetico (int **seq, int linhas, int colunas);

void populaMemoriaAG (Dados *d, ParametrosATEAMS *pATEAMS, ParametrosAG *pAG, Mkp **vmkp, int ****memoriaAG);

void genetico (ParametrosATEAMS *pATEAMS, ParametrosAG *pAG, Dados *dados, int *msize, no **principal, int *vetprob, int ****memoriaAG);

#endif /* GENETICO_H_ */
