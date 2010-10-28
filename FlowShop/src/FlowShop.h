#include "Problema.h"

#define INV_FITNESS 1000000
#define MAX_PERMUTACOES 10000

using namespace std;

#ifndef _JobShop_
#define _JobShop_

class FlowShop : public Problema
{
private:
	bool calcFitness(bool esc);		// Calcula o makespan

public:
	static char name[128];			// Nome do problema

	static short int **maq, **time;	// Matriz de maquinas e de tempos
	static int njob, nmaq;			// Quantidade de jobs e de maquinas

	static int permutacoes;			// Numero de permutacoes possiveis


	FlowShop();											// Nova solucao aleatoria
	FlowShop(short int *prob);							// Copia de prob
	FlowShop(const Problema &prob);						// Copia de prob
	FlowShop(const Problema &prob, int pos1, int pos2);	// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	virtual ~FlowShop();

	bool operator == (const Problema&);
	bool operator != (const Problema&);
	bool operator <= (const Problema&);
	bool operator >= (const Problema&);
	bool operator < (const Problema&);
	bool operator > (const Problema&);

	void imprimir(bool esc);		// Imprime o escalonamento atual

	/* Retorna um novo vizinho aleatorio */
	Problema* vizinho();

	/* Retorna um conjunto de solucoes viaveis vizinhas da atual. Retorna 'n' novos indivíduos */
	vector<pair<Problema*, movTabu*>* >* buscaLocal();	// Todos os vizinhos
	vector<pair<Problema*, movTabu*>* >* buscaLocal(float);	// Uma parcela aleatoria

	/* Faz o crossover da solucao atual com a passada como parametro. Retorna dois novos individuos */
	pair<Problema*, Problema*>* crossOver(const Problema*, int);	// Dois pivos
	pair<Problema*, Problema*>* crossOver(const Problema*);		// Um pivo

	/* Devolve uma mutacao aleatoria na solucao atual */
	Problema* mutacao(int);

	/* Devolve o valor da solucao */
	double getFitnessMaximize() const;
	double getFitnessMinimize() const;
};

void swap_vect(short int* p1, short int* p2, short int* f, int pos, int tam);

int findOrdem(int M, int maq, short int* job);

void* alocaMatriz(int, int, int, int);

void desalocaMatriz(int, void*, int, int);

bool ptcomp(pair<Problema*, movTabu*>*, pair<Problema*, movTabu*>*);

bool find(vector<Problema*> *vect, Problema *p);

#endif
