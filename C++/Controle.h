#include "JobShop.h"
#include "Tabu.h"

using namespace std;

#ifndef _Controle_
#define _Controle_

class Controle
{
public:
	Tabu* algTabu;			// Algoritmo de BuscaTabu executado

	int tamPop, numAteams;	// Tamanho da populacao e numero de iteracoes do Ateams
	multiset<Problema*, bool(*)(Problema*, Problema*)>* pop; // Populacao principal

	Controle(ParametrosATEAMS* pATEAMS, Tabu* classTabu);

	~Controle();

	/* Comeca a execucao do Ateams utilizando os algoritmos disponiveis */
	Problema* start();

	/* Gera uma populacao inicial aleatoria com 'tamPop' elementos */
	void geraPop();
};

#endif
