#include "../../Ateams_Base/src/Problem.hpp"

#define INV_FITNESS 1000000
#define MAX_PERMUTATIONS 10000

using namespace std;

#ifndef _KnapSack_
#define _KnapSack_

struct Solucao_KnapSack: public Solution {

	short int *ordemItens;		// Solucao
	short int limit;			// Ponto onde nao cabe mais itens

	friend class Problem;
	friend class KnapSack;

	friend bool fnEqualSolution(Problem*, Problem*);	// Comparacao profunda
	friend bool fnEqualFitness(Problem*, Problem*);		// Comparacao superficial
	friend bool fnSortSolution(Problem*, Problem*);		// Comparacao profunda
	friend bool fnSortFitness(Problem*, Problem*);		// Comparacao superficial
};

class InfoTabu_KnapSack: public InfoTabu {
private:

	short int A, B;

public:

	InfoTabu_KnapSack(int xA, int xB) {
		A = xA;
		B = xB;
	}

	// Verifica se 't1' eh igual a 't2'
	bool operator ==(InfoTabu &movTabu) {
		InfoTabu_KnapSack *t = dynamic_cast<InfoTabu_KnapSack*>(&movTabu);

		if ((A == t->A && B == t->B) || (A == t->B && B == t->A))
			return true;
		else
			return false;
	}
};

class KnapSack: public Problem {
protected:

	static char name[128];							// Nome do problema

	static double *values, **constraint, *limit;	// Valores, limitacoes da esquerda e direita
	static int nitens, ncontraints;					// Quantidade de itens e de limitacoes

	Solucao_KnapSack solution;						// Representacao interna da solucao

public:

	KnapSack();												// Nova solucao aleatoria
	KnapSack(short int *prob);								// Copia de prob
	KnapSack(const Problem &prob);							// Copia de prob
	KnapSack(const Problem &prob, int pos1, int pos2);		// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	~KnapSack();

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

	Solucao_KnapSack& getSoluction() {
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
