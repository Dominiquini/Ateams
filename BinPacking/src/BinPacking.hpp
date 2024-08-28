#include "../../Ateams_Base/src/Problem.hpp"

#define INV_FITNESS 1000000
#define MAX_PERMUTATIONS 10000

using namespace std;

#ifndef _BinPacking_
#define _BinPacking_

struct Solution_BinPacking: public Solution {

	short int *ordemItens;		// Ordem em que os itens serao alocados nas bolsas
	short int *bins;			// Itens para cada uma das bolsas

	friend class Problem;
	friend class BinPacking;

	friend bool fnEqualSolution(Problem*, Problem*);	// Comparacao profunda
	friend bool fnEqualFitness(Problem*, Problem*);		// Comparacao superficial
	friend bool fnSortSolution(Problem*, Problem*);		// Comparacao profunda
	friend bool fnSortFitness(Problem*, Problem*);		// Comparacao superficial
};

class InfoTabu_BinPacking: public InfoTabu {
private:

	short int A, B;

public:
	InfoTabu_BinPacking(int xA, int xB) {
		A = xA;
		B = xB;
	}

	// Verifica se 't1' eh igual a 't2'
	bool operator ==(InfoTabu &movTabu) {
		InfoTabu_BinPacking *t = dynamic_cast<InfoTabu_BinPacking*>(&movTabu);

		if ((A == t->A && B == t->B) || (A == t->B && B == t->A))
			return true;
		else
			return false;
	}
};

class BinPacking: public Problem {
protected:

	static char name[128];					// Nome do problema

	static double *sizes, capacity;			// Tamanho dos itens e capacidade de cada uma das bolsas
	static int nitens;						// Quantidade de jobs e de maquinas

	Solution_BinPacking solution;			// Representacao interna da solucao

public:

	BinPacking();											// Nova solucao aleatoria
	BinPacking(short int *prob);							// Copia de prob
	BinPacking(const Problem &prob);						// Copia de prob
	BinPacking(const Problem &prob, int pos1, int pos2);	// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	~BinPacking();

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

	Solution_BinPacking& getSoluction() {
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
