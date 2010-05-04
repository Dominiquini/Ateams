#include <sys/time.h>

#include "Problema.h"
#include "Heuristica.h"

using namespace std;

#ifndef _Controle_
#define _Controle_

class Controle
{
public:
	/* Seleciona um individuo da lista aleatoriamente, mas diretamente proporcional a sua qualidade */
	static set<Problema*, bool(*)(Problema*, Problema*)>::iterator selectRouletteWheel(set<Problema*, bool(*)(Problema*, Problema*)>* pop, int fitTotal, int randWheel);
	static vector<Problema*>::iterator selectRouletteWheel(vector<Problema*>* pop, int fitTotal, int randWheel);
	static Heuristica* selectRouletteWheel(vector<Heuristica*>* heuristc, int probTotal, int randWheel);

	vector<Heuristica*>* algs;			// Algoritmos disponiveis

	string atual;
	int makespanBest;					// Melhor makespan conhecido
	int tamPop, numAteams, maxTempo;	// Tamanho da populacao, numero de iteracoes do Ateams e tempo maximo de execucao
	set<Problema*, bool(*)(Problema*, Problema*)>* pop; // Populacao principal

	Controle();
	Controle(ParametrosATEAMS* pATEAMS);

	~Controle();

	void addHeuristic(Heuristica* alg);
	list<Problema*>* getPop();
	Problema* getSol(int n);

	/* Comeca a execucao do Ateams utilizando os algoritmos disponiveis */
	Problema* start();

private:
	/* Seleciona um dos algoritmos implementados para executar */
	vector<Problema*>* exec(int randWheel);

	/* Adiciona um novo conjunto de solucao a populacao corrente */
	int addSol(vector<Problema*> *prob);

	/* Gera uma populacao inicial aleatoria com 'tamPop' elementos */
	void geraPop();
};

bool cmpAlg(Heuristica *h1, Heuristica *h2);

#endif
