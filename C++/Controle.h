#include <sys/time.h>

#include "Problema.h"
#include "Tabu.h"

using namespace std;

#ifndef _Controle_
#define _Controle_

class Controle
{
public:
	static Problema* selectRouletteWheel(multiset<Problema*, bool(*)(Problema*, Problema*)>* pop, int fitTotal);
	static Heuristica* selectRouletteWheel(vector<Heuristica*>* heuristc, int probTotal);

	vector<Heuristica*>* algs;			// Algoritmo de BuscaTabu executado

	string atual;
	int makespanBest;					// Melhor makespan conhecido
	int tamPop, numAteams, maxTempo;	// Tamanho da populacao, numero de iteracoes do Ateams e tempo maximo de execucao
	multiset<Problema*, bool(*)(Problema*, Problema*)>* pop; // Populacao principal

	Controle(ParametrosATEAMS* pATEAMS);

	~Controle();

	void addHeuristic(Heuristica* alg);

	/* Comeca a execucao do Ateams utilizando os algoritmos disponiveis */
	Problema* start();

	/* Seleciona um dos algoritmos implementados para executar */
	vector<Problema*>* exec();

	/* Gera uma populacao inicial aleatoria com 'tamPop' elementos */
	void geraPop();
};

#endif
