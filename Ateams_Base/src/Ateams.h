#include <sys/types.h>
#include <sys/stat.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdarg.h>
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
#include <sstream>
#include <cstdlib>
#include <vector>
#include <bitset>
#include <ctime>
#include <list>
#include <set>

using namespace std;

#ifndef _ATEAMS_
#define _ATEAMS_

typedef enum ProblemType {MINIMIZACAO, MAXIMIZACAO} ProblemType;

struct Executado
{
	bool tabu;
	bool genetico;
	bool annealing;
};

struct ExecInfo
{
	double diffTime;
	int numExecs;
	long int expSol;
	double worstFitness;
	double bestFitness;
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
	float powerCrossOverAG;
	float probMutacaoAG;

	int probBT;
	int polEscolhaBT;
	int tentSemMelhoraBT;
	int iterBT;
	int tamListaBT;
	float polExplorBT;
	float funcAspiracaoBT;
	float probElitismoBT;

	int probSA;
	int polEscolhaSA;
	int maxIterSA;
	float initTempSA;
	float finalTempSA;
	int restauraSolSA;
	float alphaSA;
	float probElitismoSA;
};

void Interrompe(int signum);

int xRand(int rand, int a, int b);

int findPosArgv(char **in, int num, char *key);

float findPar(string& in, char *key);

size_t findPosPar(string& in, char *key);

#endif
