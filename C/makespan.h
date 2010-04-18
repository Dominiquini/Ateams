/*
 * makespan.h
 *
 *  Created on: 07/03/2010
 *      Author: Rafael
 */

#ifndef MAKESPAN_H_
#define MAKESPAN_H_

#include "defs.h"
#include <stdarg.h>
#include <stdio.h>

void imprimir(int N, int M, int ***esc);

int makespan(int** job, int** dur, int** seq, int N, int M, int**** escal);

int** binTOsymb(int** job, char** bin, int N, int M);
char** symbTObin(int** job, int** sym, int N, int M);

void* superMalloc(int dim, ...);
void superFree(int dim, void* mem, ...);

int** leseq(int N, int M, FILE *seq);

#endif /* MAKESPAN_H_ */
