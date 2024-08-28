#include "../../Ateams_Base/src/Problem.hpp"

#define INV_FITNESS 1000000
#define MAX_PERMUTATIONS 10000

using namespace std;

#ifndef _TravellingSalesman_
#define _TravellingSalesman_

struct Solution_TravellingSalesman: public Solution {

	short int *ordemNodes;		// Ordem em que os itens serao alocados nas bolsas

	friend class Problem;
	friend class TravellingSalesman;

	friend bool fnEqualSolution(Problem*, Problem*);	// Comparacao profunda
	friend bool fnEqualFitness(Problem*, Problem*);		// Comparacao superficial
	friend bool fnSortSolution(Problem*, Problem*);		// Comparacao profunda
	friend bool fnSortFitness(Problem*, Problem*);		// Comparacao superficial
};

class InfoTabu_TravellingSalesman: public InfoTabu {
private:

	short int A, B;

public:
	InfoTabu_TravellingSalesman(int xA, int xB) {
		A = xA;
		B = xB;
	}

	// Verifica se 't1' eh igual a 't2'
	bool operator ==(InfoTabu &movTabu) {
		InfoTabu_TravellingSalesman *t = dynamic_cast<InfoTabu_TravellingSalesman*>(&movTabu);

		if ((A == t->A && B == t->B) || (A == t->B && B == t->A))
			return true;
		else
			return false;
	}
};

class TravellingSalesman: public Problem {
protected:

	static char name[128];						// Nome do problema

	static double **edges;						// Peso das arestas que ligam os nos
	static int nnodes;							// Quantidade de nos

	Solution_TravellingSalesman solution;		// Representacao interna da solucao

public:

	TravellingSalesman();											// Nova solucao aleatoria
	TravellingSalesman(short int *prob);							// Copia de prob
	TravellingSalesman(const Problem &prob);						// Copia de prob
	TravellingSalesman(const Problem &prob, int pos1, int pos2);	// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	~TravellingSalesman();

	bool calcFitness();					// Calcula o makespan

	Solution getSolution() override {	// Retorna solucao
		return solution;
	}

	/* Imprime o escalonamento atual */
	void print(bool esc) override;

	/* Retorna um novo vizinho aleatorio */
	Problem* neighbor() override;

	/* Retorna um conjunto de solucoes viaveis vizinhas da atual. Retorna 'n' novos indivíduos */
	vector<pair<Problem*, InfoTabu*>*>* localSearch() override;				// Todos os vizinhos
	vector<pair<Problem*, InfoTabu*>*>* localSearch(float) override;		// Uma parcela aleatoria

	/* Faz o crossover da solucao atual com a passada como parametro. Retorna dois novos individuos */
	pair<Problem*, Problem*>* crossOver(const Problem*, int, int) override;	// Dois pivos
	pair<Problem*, Problem*>* crossOver(const Problem*, int) override;		// Um pivo

	/* Devolve uma mutacao aleatoria na solucao atual */
	Problem* mutation(int) override;

	/* Devolve o valor da solucao */
	double getFitness() const override;
	double getFitnessMaximize() const override;
	double getFitnessMinimize() const override;

	Solution_TravellingSalesman& getSoluction() {
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
