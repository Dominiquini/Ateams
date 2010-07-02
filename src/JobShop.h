#include "Problema.h"

#define INV_FITNESS 1000000

using namespace std;

#ifndef _JobShop_
#define _JobShop_

class JobShop : public Problema
{
protected:
	int calcMakespan(bool esc);		// Calcula o makespan

public:
	static char name[128];			// Nome do problema
	static short int **maq, **time;	// Matriz de maquinas e de tempos
	static int njob, nmaq;			// Quantidade de jobs e de maquinas

	JobShop();													// Nova solucao aleatoria
	JobShop(short int **prob);									// Copia de prob
	JobShop(const Problema &prob);								// Copia de prob
	JobShop(const Problema &prob, int maq, int pos1, int pos2);	// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	~JobShop();

	bool operator == (Problema&);
	bool operator != (Problema&);
	bool operator <= (Problema&);
	bool operator >= (Problema&);
	bool operator < (Problema&);
	bool operator > (Problema&);

	void imprimir(bool esc);		// Imprime o escalonamento atual

	/* Retorna um novo vizinho aleatorio */
	Problema* vizinho();

	/* Retorna um conjunto de solucoes viaveis vizinhas da atual. Retorna 'n' novos indivíduos */
	vector<pair<Problema*, movTabu*>* >* buscaLocal();	// Todos os vizinhos
	vector<pair<Problema*, movTabu*>* >* buscaLocal(float);	// Uma parcela aleatoria

	/* Faz o crossover da solucao atual com a passada como parametro. Retorna dois novos individuos */
	pair<Problema*, Problema*>* crossOver(Problema*, int);	// Dois pivos
	pair<Problema*, Problema*>* crossOver(Problema*);		// Um pivo

	/* Devolve uma mutacao aleatoria na solucao atual */
	Problema* mutacao(int);

	/* Devolve o valor da solucao */
	double getFitnessMaximize();
	double getFitnessMinimize();

	/* Devolve a representacao interna da solucao */
	soluction* getSoluction();
};

void swap_vect(short int* p1, short int* p2, short int* f, int pos, int tam);

int findOrdem(int M, int maq, short int* job);

void* alocaMatriz(int, int, int, int);

void desalocaMatriz(int, void*, int, int);

bool ptcomp(pair<Problema*, movTabu*>*, pair<Problema*, movTabu*>*);

bool find(vector<Problema*> *vect, Problema *p);

#endif
