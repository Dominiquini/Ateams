#include "Ateams.hpp"
#include "Control.hpp"

using namespace std;

extern pthread_mutex_t mutex_cont;

#ifndef _PROBLEM_
#define _PROBLEM_

struct ExecutionInfo;

class InfoTabu;

enum ProblemType {
	MINIMIZATION, MAXIMIZATION
};

class Solution {
protected:

	double fitness;		// Fitness da solucao
};

class Problem {
public:

	static ProblemType TYPE;

	static double best;				// Melhor solucao do momento
	static double worst;			// Pior solucao do momento
	static int numInst;				// Quantidade de instancias criadas
	static long int totalNumInst;	// Quantidade total de problemas processados

	// Le arquivo de dados de entrada
	static void readProblemFromFile(char*);

	// Le a especificacao do problema
	static list<Problem*>* readPopulationFromLog(char*);

	// Imprime em um arquivo os resultados da execucao
	static void writeCurrentPopulationInLog(char*, list<Problem*>*);
	static void writeResultInFile(char*, char*, ExecutionInfo*, char*);

	// Aloca e Desaloca as estruturas do problema
	static void allocateMemory();
	static void deallocateMemory();

	// Alocador generico
	static Problem* randomSolution();					// Nova solucao aleatoria
	static Problem* copySolution(const Problem &prob);	// Copia de prob

	// Calcula a melhora (Resultado Positivo) de newP em relacao a oldP
	static double improvement(Problem &oldP, Problem &newP) {
		return improvement(oldP.getFitness(), newP.getFitness());
	}

	static double improvement(double oldP, double newP) {
		if (TYPE == MINIMIZATION)
			return oldP - newP;
		else
			return newP - oldP;
	}

	ExecHeuristicsInfo exec;							// Algoritmos executados na solucao

	// Contrutor/Destrutor padrao: Incrementa ou decrementa um contador de instancias
	Problem() {
		pthread_mutex_lock(&mutex_cont);
		numInst++;
		totalNumInst++;
		pthread_mutex_unlock(&mutex_cont);
	}	// numInst++
	virtual ~Problem() {
		pthread_mutex_lock(&mutex_cont);
		numInst--;
		pthread_mutex_unlock(&mutex_cont);
	}		// numInst--

	virtual void print(bool esc) = 0;	// Imprime o escalonamento

	/* Retorna um vizinho aleatorio da solucao atual */
	virtual Problem* neighbor() = 0;

	/* Retorna um conjunto de solucoes viaveis vizinhas da atual */
	virtual vector<pair<Problem*, InfoTabu*>*>* localSearch() = 0;		// Todos os vizinhos
	virtual vector<pair<Problem*, InfoTabu*>*>* localSearch(float) = 0;	// Uma parcela aleatoria

	/* Realiza um crossover com uma outra solucao */
	virtual pair<Problem*, Problem*>* crossOver(const Problem*, int, int) = 0;	// Dois pivos
	virtual pair<Problem*, Problem*>* crossOver(const Problem*, int) = 0;			// Um pivo

	/* Devolve uma mutacao aleatoria na solucao atual */
	virtual Problem* mutation(int) = 0;

	/* Devolve o valor da solucao */
	virtual double getFitness() const = 0;
	virtual double getFitnessMaximize() const = 0;	// Problemas de Maximizacao
	virtual double getFitnessMinimize() const = 0;	// Problemas de Minimizacao

private:

	virtual bool calcFitness(bool esc) = 0;		// Calcula o makespan

	friend bool fnequal1(Problem*, Problem*);	// Comparacao profunda
	friend bool fnequal2(Problem*, Problem*);	// Comparacao superficial
	friend bool fncomp1(Problem*, Problem*);	// Comparacao profunda
	friend bool fncomp2(Problem*, Problem*);	// Comparacao superficial
};

bool fncomp1(Problem*, Problem*);	//Se P1 for menor que P2
bool fncomp2(Problem*, Problem*); 	//Se P1 for menor que P2, considerando apenas o fitness
bool fnequal1(Problem*, Problem*);	//Se P1 for igual a P2
bool fnequal2(Problem*, Problem*);	//Se P1 for igual a P2, considerando apenas o fitness

#endif