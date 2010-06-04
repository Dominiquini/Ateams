#include "Problema.h"
#include "Heuristica.h"

using namespace std;

#ifndef _Controle_
#define _Controle_

class Controle
{
private:
	static void* run(void *obj);

public:
	/* Seleciona um individuo da lista aleatoriamente, mas diretamente proporcional a sua qualidade */
	static set<Problema*, bool(*)(Problema*, Problema*)>::iterator selectRouletteWheel(set<Problema*, bool(*)(Problema*, Problema*)>* pop, double fitTotal, unsigned int randWheel);
	static vector<Problema*>::iterator selectRouletteWheel(vector<Problema*>* pop, double fitTotal, unsigned int randWheel);
	static Heuristica* selectRouletteWheel(vector<Heuristica*>* heuristc, unsigned int probTotal, unsigned int randWheel);

	/* Seleciona um individuo aleatoriamente */
	static vector<Problema*>::iterator selectRandom(vector<Problema*>* pop, int randWheel);

private:
	vector<Heuristica*>* algs;			// Algoritmos disponiveis

	int numThreads;						// Número de threads que podem rodar ao mesmo tempo
	int makespanBest;					// Melhor makespan conhecido
	int tamPop, iterAteams, maxTempo;	// Tamanho da populacao, numero de iteracoes do Ateams e tempo maximo de execucao
	set<Problema*, bool(*)(Problema*, Problema*)>* pop; // Populacao principal

	/* Seleciona um dos algoritmos implementados para executar */
	pair<vector<Problema*>*, string*>* exec(int randWheel);

	/* Adiciona um novo conjunto de solucao a populacao corrente */
	int addSol(vector<Problema*> *news);

	/* Gera uma populacao inicial aleatoria com 'tamPop' elementos */
	void geraPop();

public:
	Controle();
	Controle(ParametrosATEAMS* pATEAMS);

	~Controle();

	void addHeuristic(Heuristica* alg);
	list<Problema*>* getPop();
	Problema* getSol(int n);

	/* Comeca a execucao do Ateams utilizando os algoritmos disponiveis */
	Problema* start();
};

bool cmpAlg(Heuristica *h1, Heuristica *h2);

#endif
