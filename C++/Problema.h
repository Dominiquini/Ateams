#include <stdio.h>

#include <functional>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <list>
#include <set>

using namespace std;

#ifndef _PROBLEMA_
#define _PROBLEMA_


typedef struct movTabu
{
	int maq, A, B;
} mov;

class Problema;

bool fncomp(Problema*, Problema*);

class Problema
{
public:
	static int numInst;

	static char name[128];
	static int **maq, **time;
	static int njob, nmaq;

	static void leProblema(FILE*);


	mov movTabu;

	int **esc;
	int makespan;
	int ***escalon;

	Problema();

	~Problema();

	virtual int calcMakespan() = 0;
	virtual void imprimir() = 0;

	virtual multiset<Problema*, bool(*)(Problema*, Problema*)>* buscaLocal() = 0;
};

#endif
