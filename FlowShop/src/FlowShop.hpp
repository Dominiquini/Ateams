#include "../../Ateams_Base/src/Problem.hpp"

#define INV_FITNESS 1000000
#define MAX_PERMUTATIONS 10000

using namespace std;

#ifndef _FlowShop_
#define _FlowShop_

struct Solution_FlowShop: public Solution {

	short int *scaling;			// Solucao
	short int ***staggering;	// Escalonamento nas maquinas - Grafico de Gant

	friend class Problem;
	friend class FlowShop;

	friend bool fnEqualSolution(Problem*, Problem*);	// Comparacao profunda
	friend bool fnEqualFitness(Problem*, Problem*);		// Comparacao superficial
	friend bool fnSortSolution(Problem*, Problem*);		// Comparacao profunda
	friend bool fnSortFitness(Problem*, Problem*);		// Comparacao superficial
};

class InfoTabu_FlowShop: public InfoTabu {
private:

	short int A, B;

public:
	InfoTabu_FlowShop(int xA, int xB) {
		A = xA;
		B = xB;
	}

	// Verifica se 't1' eh igual a 't2'
	bool operator ==(InfoTabu &movTabu) {
		InfoTabu_FlowShop *t = dynamic_cast<InfoTabu_FlowShop*>(&movTabu);

		if ((A == t->A && B == t->B) || (A == t->B && B == t->A))
			return true;
		else
			return false;
	}
};

class FlowShop: public Problem {
protected:

	static char name[128];					// Nome do problema

	static int **time;						// Matriz de maquinas e de tempos
	static int njob, nmaq;					// Quantidade de jobs e de maquinas

	Solution_FlowShop solution;				// Representacao interna da solucao

public:

	FlowShop();											// Nova solucao aleatoria
	FlowShop(short int *prob);							// Copia de prob
	FlowShop(const Problem &prob);						// Copia de prob
	FlowShop(const Problem &prob, int pos1, int pos2);	// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	~FlowShop();

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

	Solution_FlowShop& getSoluction() {
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
