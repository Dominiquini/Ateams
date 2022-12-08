#include "../../Ateams_Base/src/Problem.hpp"

#define INV_FITNESS 1000000
#define MAX_PERMUTATIONS 10000

using namespace std;

#ifndef _GraphColoring_
#define _GraphColoring_

struct Solution_GraphColoring: public Solution {

	short int *ordemNodes;		// Ordem em que os nos serao coloridos
	short int *colors;			// Cores de cada um dos nos

	friend class Problem;
	friend class GraphColoring;

	friend bool fnEqualSolution(Problem*, Problem*);	// Comparacao profunda
	friend bool fnEqualFitness(Problem*, Problem*);		// Comparacao superficial
	friend bool fnSortSolution(Problem*, Problem*);		// Comparacao profunda
	friend bool fnSortFitness(Problem*, Problem*);		// Comparacao superficial
};

class InfoTabu_GraphColoring: public InfoTabu {
private:

	short int A, B;

public:
	InfoTabu_GraphColoring(int xA, int xB) {
		A = xA;
		B = xB;
	}

	// Verifica se 't1' eh igual a 't2'
	bool operator ==(InfoTabu &movTabu) {
		InfoTabu_GraphColoring *t = dynamic_cast<InfoTabu_GraphColoring*>(&movTabu);

		if ((A == t->A && B == t->B) || (A == t->B && B == t->A))
			return true;
		else
			return false;
	}
};

class GraphColoring: public Problem {
protected:

	static char name[128];					// Nome do problema

	static vector<int> **edges;				// Matriz com as arestas
	static int nedges, nnodes;				// Quantidade de arestas e de nos

	Solution_GraphColoring solution;		// Representacao interna da solucao

public:

	GraphColoring();										// Nova solucao aleatoria
	GraphColoring(short int *prob);							// Copia de prob
	GraphColoring(const Problem &prob);						// Copia de prob
	GraphColoring(const Problem &prob, int pos1, int pos2);	// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	~GraphColoring();

	bool calcFitness();					// Calcula o makespan

	Solution getSolution() override {	// Retorna solucao
		return solution;
	}

	void print(bool esc);		// Imprime o escalonamento atual

	/* Retorna um novo vizinho aleatorio */
	Problem* neighbor();

	/* Retorna um conjunto de solucoes viaveis vizinhas da atual. Retorna 'n' novos indivíduos */
	vector<pair<Problem*, InfoTabu*>*>* localSearch();			// Todos os vizinhos
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

	Solution_GraphColoring& getSoluction() {
		return solution;
	}

	friend class Problem;

	friend void swap_vect(short int *p1, short int *p2, short int *f, int pos, int tam);

	friend bool fnEqualSolution(Problem*, Problem*);	// Comparacao profunda
	friend bool fnEqualFitness(Problem*, Problem*);		// Comparacao superficial
	friend bool fnSortSolution(Problem*, Problem*);		// Comparacao profunda
	friend bool fnSortFitness(Problem*, Problem*);		// Comparacao superficial
};

void swap_vect(short int *p1, short int *p2, short int *f, int pos, int tam);

#endif
