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
#include <random>
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

int xRand();

int xRand(int min, int max);

void terminate(int signal);

#endif
