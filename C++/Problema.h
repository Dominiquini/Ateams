#include <functional>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <list>
#include <set>

#include "Ateams.h"

using namespace std;

#ifndef _PROBLEMA_
#define _PROBLEMA_

extern pthread_mutex_t mut_p;

class Problema;

bool fncomp1(Problema*, Problema*);
bool fncomp2(Problema*, Problema*);
bool fnequal(Problema*, Problema*);

class Problema
{
protected:
	soluction sol;	// Makespan e escalonamentos que definem a solucao

public:
	static int best;				// Melhor solucao do momento
	static int worst;				// Pior solucao do momento
	static int numInst;				// Quantidade de instancias criadas

	// Le arquivo de dados de entrada
	static void leProblema(FILE*);

	// Le arquivo de parametros de entrada
	static void leParametros(FILE*, ParametrosATEAMS*, ParametrosBT*, ParametrosAG*, ParametrosSA*);
	static void leArgumentos(char**, int, ParametrosATEAMS*, ParametrosBT*, ParametrosAG*, ParametrosSA*);

	static void imprimeResultado (struct timeval, struct timeval, FILE*, int);

	static void desalocaMemoria();

	static Problema* alloc();													// Nova solucao aleatoria
	static Problema* alloc(short int **prob);									// Copia de prob
	static Problema* alloc(const Problema &prob);								// Copia de prob
	static Problema* alloc(const Problema &prob, int maq, int pos1, int pos2);	// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	static bool movTabuCMP(tTabu& t1, tTabu& t2);
	static double sumFitnessMaximize(set<Problema*, bool(*)(Problema*, Problema*)> *pop, int n);
	static double sumFitnessMaximize(vector<Problema*> *pop, int n);

	Problema() {pthread_mutex_lock(&mut_p); numInst++; pthread_mutex_unlock(&mut_p);}			// numInst++
	virtual ~Problema() {pthread_mutex_lock(&mut_p); numInst--; pthread_mutex_unlock(&mut_p);}	// numInst--

	virtual int calcMakespan() = 0;			// Calcula o makespan
	virtual void imprimir(bool esc) = 0;	// Imprime o escalonamento

	/* Retorna um vizinho aleatorio */
	virtual Problema* vizinho() = 0;

	/* Retorna um conjunto de solucoes viaveis vizinhas da atual */
	virtual vector<pair<Problema*, tTabu*>* >* buscaLocal() = 0;	// Todos os vizinhos
	virtual vector<pair<Problema*, tTabu*>* >* buscaLocal(float) = 0;	// Uma parcela aleatoria

	/* Realiza um crossover com uma outra solucao */
	virtual pair<Problema*, Problema*>* crossOver(Problema*, int) = 0;	// Dois pivos
	virtual pair<Problema*, Problema*>* crossOver(Problema*) = 0;		// Um pivo

	/* Provoca uma mutacao aleatoria na solucao */
	virtual void mutacao() = 0;

	virtual double getFitnessMaximize() = 0;
	virtual double getFitnessMinimize() = 0;

	executado exec;

	friend class JobShop;
	friend bool fnequal(Problema*, Problema*);
	friend bool fncomp1(Problema*, Problema*);
	friend bool fncomp2(Problema*, Problema*);
	friend bool vtcomp(pair<Problema*, tTabu*>*, pair<Problema*, tTabu*>*);
};

#endif
