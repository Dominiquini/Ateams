#include "../../Ateams_Base/src/Problem.hpp"

#define INV_FITNESS 1000000
#define MAX_PERMUTATIONS 10000

using namespace std;

#ifndef _JobShop_
#define _JobShop_

class Solution_JobShop: public Solution {
private:

	short int **scaling;		// Solucao
	short int ***staggering;	// Escalonamento nas maquinas - Grafico de Gant

	friend class Problem;
	friend class JobShop;

	friend bool fnequal1(Problem*, Problem*);	// Comparacao profunda
	friend bool fnequal2(Problem*, Problem*);	// Comparacao superficial
	friend bool fncomp1(Problem*, Problem*);	// Comparacao profunda
	friend bool fncomp2(Problem*, Problem*);	// Comparacao superficial
};

class InfoTabu_JobShop: public InfoTabu {
private:

	short int maq, A, B;

public:

	InfoTabu_JobShop(int xmaq, int xA, int xB) {
		maq = xmaq;
		A = xA;
		B = xB;
	}

	// Verifica se 't1' eh igual a 't2'
	bool operator ==(InfoTabu &movTabu) {
		InfoTabu_JobShop *t = dynamic_cast<InfoTabu_JobShop*>(&movTabu);

		if ((maq == t->maq) && ((A == t->A && B == t->B) || (A == t->B && B == t->A)))
			return true;
		else
			return false;
	}
};

class JobShop: public Problem {
private:

	bool calcFitness();				// Calcula o makespan

	Solution_JobShop solution;		// Representacao interna da solucao

public:

	static char name[128];			// Nome do problema

	static int **maq, **time;		// Matriz de maquinas e de tempos
	static int njob, nmaq;			// Quantidade de jobs e de maquinas

	static int neighbors;			// Numero de permutacoes possiveis

	JobShop();													// Nova solucao aleatoria
	JobShop(short int **prob);									// Copia de prob
	JobShop(const Problem &prob);								// Copia de prob
	JobShop(const Problem &prob, int maq, int pos1, int pos2);	// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	~JobShop();

	void print(bool esc);		// Imprime o escalonamento atual

	/* Retorna um novo vizinho aleatorio */
	Problem* neighbor();

	/* Retorna um conjunto de solucoes viaveis vizinhas da atual. Retorna 'n' novos indivíduos */
	vector<pair<Problem*, InfoTabu*>*>* localSearch();		// Todos os vizinhos
	vector<pair<Problem*, InfoTabu*>*>* localSearch(float);	// Uma parcela aleatoria

	/* Faz o crossover da solucao atual com a passada como parametro. Retorna dois novos individuos */
	pair<Problem*, Problem*>* crossOver(const Problem*, int, int);	// Dois pivos
	pair<Problem*, Problem*>* crossOver(const Problem*, int);		// Um pivo

	/* Devolve uma mutacao aleatoria na solucao atual */
	Problem* mutation(int);

	/* Devolve o valor da solucao */
	double getFitness() const;
	double getFitnessMaximize() const;
	double getFitnessMinimize() const;

	Solution_JobShop& getSoluction() {
		return solution;
	}

	friend bool fnequal1(Problem*, Problem*);	// Comparacao profunda
	friend bool fnequal2(Problem*, Problem*);	// Comparacao superficial
	friend bool fncomp1(Problem*, Problem*);	// Comparacao profunda
	friend bool fncomp2(Problem*, Problem*);	// Comparacao superficial
};

void swap_vect(short int *p1, short int *p2, short int *f, int pos, int tam);

bool ptcomp(pair<Problem*, InfoTabu*>*, pair<Problem*, InfoTabu*>*);

bool find(vector<Problem*> *vect, Problem *p);

#endif
