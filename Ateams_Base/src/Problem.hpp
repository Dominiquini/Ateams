#include "Control.hpp"

#include "FileUtils.hpp"

using namespace std;

#ifndef _PROBLEM_
#define _PROBLEM_

extern mutex mutex_counter;

struct ExecutionInfo;

class InfoTabu;

class Problem;

enum ProblemType {
	MINIMIZATION, MAXIMIZATION
};

struct Solution {

	double fitness = -1;								// Fitness da solucao

	friend class Problem;

	friend bool fnEqualSolution(Problem*, Problem*);	// Comparacao profunda
	friend bool fnEqualFitness(Problem*, Problem*);		// Comparacao superficial
	friend bool fnSortSolution(Problem*, Problem*);		// Comparacao profunda
	friend bool fnSortFitness(Problem*, Problem*);		// Comparacao superficial
};

class Problem {
public:

	static ProblemType TYPE;

	static double best;							// Melhor solucao do momento
	static double worst;						// Pior solucao do momento

	static unsigned int numInst;				// Quantidade de instancias criadas
	static unsigned long long totalNumInst;		// Quantidade total de problemas processados

	static unsigned int neighbors;				// Numero de permutacoes possiveis

	// Le arquivo de dados de entrada
	static void readProblemFromFile(char*);

	// Le a especificacao do problema
	static list<Problem*>* readPopulationFromLog(char*);

	// Imprime em um arquivo os resultados da execucao
	static void writeCurrentPopulationInLog(char*, list<Problem*>*);
	static void writeResultInFile(char*, char*, char*, ExecutionInfo*);

	// Aloca e Desaloca as estruturas do problema
	static void allocateMemory();
	static void deallocateMemory();

	// Alocador generico
	static Problem* randomSolution();					// Nova solucao aleatoria
	static Problem* copySolution(const Problem &prob);	// Copia de prob

	// Calcula a melhora (Resultado Positivo) de newP em relacao a oldP
	static inline double improvement(Problem &oldP, Problem &newP) {
		return improvement(oldP.getFitness(), newP.getFitness());
	}

	static inline double improvement(double oldP, double newP) {
		switch(TYPE) {
			case MINIMIZATION: return oldP - newP;
			case MAXIMIZATION: return newP - oldP;
			default: return 0;
		}
	}

	/* Retorna a soma de fitness de uma populacao */
	static inline double sumFitnessMaximize(set<Problem*, bool (*)(Problem*, Problem*)> *probs, int n) {
		set<Problem*>::const_iterator iter;
		double sum = 0, i = 0;

		for (i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++)
			sum += (*iter)->getFitnessMaximize();

		return sum;
	}

	/* Retorna a soma de fitness de uma populacao */
	static inline double sumFitnessMaximize(vector<Problem*> *probs, int n) {
		vector<Problem*>::const_iterator iter;
		double sum = 0, i = 0;

		for (i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++)
			sum += (*iter)->getFitnessMaximize();

		return sum;
	}

	/* Retorna a soma de fitness de uma populacao */
	static inline double sumFitnessMinimize(set<Problem*, bool (*)(Problem*, Problem*)> *probs, int n) {
		set<Problem*>::const_iterator iter;
		double sum = 0, i = 0;

		for (i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++)
			sum += (*iter)->getFitnessMinimize();

		return sum;
	}

	/* Retorna a soma de fitness de uma populacao */
	static inline double sumFitnessMinimize(vector<Problem*> *probs, int n) {
		vector<Problem*>::const_iterator iter;
		double sum = 0, i = 0;

		for (i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++) {
			sum += (*iter)->getFitnessMinimize();
		}

		return sum;
	}

	/* Seleciona uma solucao aleatoriamente */
	static inline vector<Problem*>::iterator selectRandomSolution(vector<Problem*> *population) {
		unsigned int randWheel = Random::randomNumber(0, population->size());

		vector<Problem*>::iterator iter = population->begin();

		advance(iter, randWheel);

		return iter;
	}

	/* Seleciona uma solucao da lista aleatoriamente, mas diretamente proporcional a sua qualidade */
	static inline set<Problem*>::const_iterator selectOpportunisticSolution(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) {
		unsigned int sum = (unsigned int) fitTotal;
		unsigned int randWheel = Random::randomNumber(0, sum);

		set<Problem*>::const_iterator iter;
		for (iter = population->cbegin(); iter != population->cend(); iter++) {
			sum -= (int) (*iter)->getFitnessMaximize();
			if (sum <= randWheel) {
				return iter;
			}
		}

		return population->begin();
	}

	/* Seleciona uma solucao da lista aleatoriamente, mas diretamente proporcional a sua qualidade */
	static inline vector<Problem*>::const_iterator selectOpportunisticSolution(vector<Problem*> *population, double fitTotal) {
		unsigned int sum = (unsigned int) fitTotal;
		unsigned int randWheel = Random::randomNumber(0, sum);

		vector<Problem*>::const_iterator iter;
		for (iter = population->cbegin(); iter != population->cend(); iter++) {
			sum -= (int) (*iter)->getFitnessMaximize();
			if (sum <= randWheel) {
				return iter;
			}
		}

		return population->begin();
	}

	static inline list<Problem*>::iterator findSolution(list<Problem*> *vect, Problem *p) {
		list<Problem*>::iterator iter;

		for (iter = vect->begin(); iter != vect->end(); iter++)
			if (fnEqualSolution((*iter), p))
				return iter;

		return iter;
	}

	static inline bool ptcomp(pair<Problem*, InfoTabu*> *p1, pair<Problem*, InfoTabu*> *p2) {
		switch(TYPE) {
			case MINIMIZATION: return (p1->first->getFitness() > p2->first->getFitness());
			case MAXIMIZATION: return (p1->first->getFitness() < p2->first->getFitness());
			default: return false;
		}
	}

public:

	HeuristicsCounter heuristicsCounter;			// Algoritmos executados na solucao

	// Contrutor/Destrutor padrao: Incrementa ou decrementa um contador de instancias
	Problem() noexcept(false) {
		scoped_lock<decltype(mutex_counter)> lock(mutex_counter);

		numInst++;
		totalNumInst++;
	}
	virtual ~Problem() noexcept(false) {
		scoped_lock<decltype(mutex_counter)> lock(mutex_counter);

		numInst--;
	}

	bool calcFitness();					// Calcula o makespan

	virtual Solution getSolution() = 0;	// Retorna solucao

	virtual void print(bool esc) = 0;	// Imprime o escalonamento

	/* Retorna um vizinho aleatorio da solucao atual */
	virtual Problem* neighbor() = 0;

	/* Retorna um conjunto de solucoes viaveis vizinhas da atual */
	virtual vector<pair<Problem*, InfoTabu*>*>* localSearch() = 0;		// Todos os vizinhos
	virtual vector<pair<Problem*, InfoTabu*>*>* localSearch(float) = 0;	// Uma parcela aleatoria

	/* Realiza um crossover com uma outra solucao */
	virtual pair<Problem*, Problem*>* crossOver(const Problem*, int, int) = 0;	// Dois pivos
	virtual pair<Problem*, Problem*>* crossOver(const Problem*, int) = 0;		// Um pivo

	/* Devolve uma mutacao aleatoria na solucao atual */
	virtual Problem* mutation(int) = 0;

	/* Devolve o valor da solucao */
	virtual double getFitness() const = 0;
	virtual double getFitnessMaximize() const = 0;	// Problemas de Maximizacao
	virtual double getFitnessMinimize() const = 0;	// Problemas de Minimizacao

	friend bool fnEqualSolution(Problem*, Problem*);	// Comparacao profunda
	friend bool fnEqualFitness(Problem*, Problem*);		// Comparacao superficial
	friend bool fnSortSolution(Problem*, Problem*);		// Comparacao profunda
	friend bool fnSortFitness(Problem*, Problem*);		// Comparacao superficial
};

bool fnEqualSolution(Problem*, Problem*);	//Se P1 for igual a P2
bool fnEqualFitness(Problem*, Problem*);	//Se P1 for igual a P2, considerando apenas o fitness
bool fnSortSolution(Problem*, Problem*);	//Se P1 for menor que P2
bool fnSortFitness(Problem*, Problem*); 	//Se P1 for menor que P2, considerando apenas o fitness

template<typename T>
T* allocateMatrix(int dim, int a, int b = 1, int c = 1) {
	if (dim == 1) {
		T *M = (T*) malloc(a * sizeof(T));

		return (T*) M;
	} else if (dim == 2) {
		T **M = (T**) malloc(a * sizeof(T*));

		for (int i = 0; i < a; i++) {
			M[i] = (T*) malloc(b * sizeof(T));
		}

		return (T*) M;
	} else if (dim == 3) {
		T ***M = (T***) malloc(a * sizeof(T**));

		for (int i = 0; i < a; i++) {
			M[i] = (T**) malloc(b * sizeof(T*));
			for (int j = 0; j < b; j++) {
				M[i][j] = (T*) malloc(c * sizeof(T));
			}
		}

		return (T*) M;
	} else
		return NULL;
}

template<typename T>
void deallocateMatrix(int dim, void *MMM, int a = 1, int b = 1) {
	if (MMM != NULL) {
		if (dim == 1) {
			T *M = (T*) MMM;

			free(M);
		} else if (dim == 2) {
			T **M = (T**) MMM;

			for (int i = 0; i < a; i++) {
				free(M[i]);
			}

			free(M);
		} else if (dim == 3) {
			T ***M = (T***) MMM;

			for (int i = 0; i < a; i++) {
				for (int j = 0; j < b; j++) {
					free(M[i][j]);
				}

				free(M[i]);
			}

			free(M);
		}
	}

	return;
}

#endif
