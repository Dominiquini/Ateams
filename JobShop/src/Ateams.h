#include <signal.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <pthread.h>
#include <semaphore.h>

#include <functional>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <list>
#include <set>

using namespace std;

#ifndef _ATEAMS_
#define _ATEAMS_

typedef struct soluction
{
	int makespan;			// Makespan da solucao
	short int **esc;		// Solucao
	short int ***escalon;	// Escalonamento nas maquinas - Grafico de Gant
} soluction;

typedef struct executado
{
	bool tabu;
	bool genetico;
	bool annealing;
} executado;

typedef struct execInfo
{
	double diffTime;
	int numExecs;
	long int expSol;
	double worstFitness;
	double bestFitness;
} execInfo;

struct movTabu
{
	short int maq, A, B;

	movTabu(int xmaq, int xA, int xB)
	{
		maq = xmaq;
		A = xA;
		B = xB;
	}

	// Verifica se 't1' eh igual a 't2'
	bool operator == (movTabu& t)
	{
		if((maq == t.maq) && ((A == t.A && B == t.B) || (A == t.B && B == t.A)))
			return true;
		else
			return false;
	}
};

struct ParametrosATEAMS {
	int tamPopAteams;
	int critUnicidade;
	int iterAteams;
	int numThreads;
	int tentAteams;
	int maxTempoAteams;
	int makespanBest;
};

enum Heuristicas {SA, BT, AG};

struct ParametrosHeuristicas {
	Heuristicas alg;
	char algName[16];

	int probAG;
	int polEscolhaAG;
	int iterAG;
	int tamPopAG;
	int tamParticaoAG;
	float probCrossOverAG;
	float probMutacaoAG;

	int probBT;
	int polEscolhaBT;
	int tentSemMelhoraBT;
	int iterBT;
	int tamListaBT;
	float polExplorBT;
	float funcAspiracaoBT;

	int probSA;
	int polEscolhaSA;
	int maxIterSA;
	float initTempSA;
	float finalTempSA;
	int restauraSolSA;
	float alphaSA;
};

void Interrompe(int signum);

int xRand(int rand, int a, int b);

int findPosArgv(char **in, int num, char *key);

float findPar(string& in, char *key);

size_t findPosPar(string& in, char *key);

#endif