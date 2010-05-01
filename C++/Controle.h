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
	static Problema* selectRouletteWheel(vector<Problema*>* pop, int fitTotal, int randWheel);
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

	/* Comeca a execucao do Ateams utilizando os algoritmos disponiveis */
	Problema* start();

	/* Seleciona um dos algoritmos implementados para executar */
	vector<Problema*>* exec(int randWheel);

	/* Gera uma populacao inicial aleatoria com 'tamPop' elementos */
	void geraPop();
};

#endif
