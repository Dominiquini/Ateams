#include "Ateams.hpp"
#include "Controle.hpp"

using namespace std;

extern pthread_mutex_t mutex_cont;

#ifndef _PROBLEMA_
#define _PROBLEMA_

class Problema;
class InfoTabu;
class Solucao;

bool fncomp1(Problema*, Problema*);	//Se P1 for menor que P2
bool fncomp2(Problema*, Problema*); 	//Se P1 for menor que P2, considerando apenas o fitness
bool fnequal1(Problema*, Problema*);	//Se P1 for igual a P2
bool fnequal2(Problema*, Problema*);	//Se P1 for igual a P2, considerando apenas o fitness

class Solucao
{
protected:

	double fitness;		// Fitness da solucao
};

class InfoTabu
{
public:

	virtual bool operator == (InfoTabu&) = 0;

	InfoTabu() {}
	virtual ~InfoTabu() {}
};

class Problema
{
public:

	static ProblemType TIPO;

	static double best;				// Melhor solucao do momento
	static double worst;			// Pior solucao do momento
	static int numInst;				// Quantidade de instancias criadas
	static long int totalNumInst;	// Quantidade total de problemas processados

	// Le arquivo de dados de entrada
	static void leProblema(char*);

	// Le a especificacao do problema
	static list<Problema*>* lePopulacao(char*);

	// Imprime em um arquivo os resultados da execucao
	static void escrevePopulacao(char*, list<Problema*>*);
	static void escreveResultado(char*, char*, ExecInfo*, char*);

	// Aloca e Desaloca as estruturas do problema
	static void alocaMemoria();
	static void desalocaMemoria();

	// Alocador generico
	static Problema* randSoluction();							// Nova solucao aleatoria
	static Problema* copySoluction(const Problema& prob);	// Copia de prob

	// Calcula a melhora (Resultado Positivo) de newP em relacao a oldP
	static double melhora(double oldP, double newP)
	{
		if(TIPO == MINIMIZACAO)
			return oldP - newP;
		else
			return newP - oldP;
	}

	static double melhora(Problema& oldP, Problema& newP)
	{
		if(TIPO == MINIMIZACAO)
			return oldP.getFitness() - newP.getFitness();
		else
			return newP.getFitness() - oldP.getFitness();
	}


	Executado exec;							// Algoritmos executados na solucao

	// Contrutor/Destrutor padrao: Incrementa ou decrementa um contador de instancias
	Problema() {pthread_mutex_lock(&mutex_cont); numInst++; totalNumInst++; pthread_mutex_unlock(&mutex_cont);}	// numInst++
	virtual ~Problema() {pthread_mutex_lock(&mutex_cont); numInst--; pthread_mutex_unlock(&mutex_cont);}		// numInst--

	virtual void imprimir(bool esc) = 0;	// Imprime o escalonamento

	/* Retorna um vizinho aleatorio da solucao atual */
	virtual Problema* vizinho() = 0;

	/* Retorna um conjunto de solucoes viaveis vizinhas da atual */
	virtual vector<pair<Problema*, InfoTabu*>* >* buscaLocal() = 0;		// Todos os vizinhos
	virtual vector<pair<Problema*, InfoTabu*>* >* buscaLocal(float) = 0;	// Uma parcela aleatoria

	/* Realiza um crossover com uma outra solucao */
	virtual pair<Problema*, Problema*>* crossOver(const Problema*, int, int) = 0;	// Dois pivos
	virtual pair<Problema*, Problema*>* crossOver(const Problema*, int) = 0;			// Um pivo

	/* Devolve uma mutacao aleatoria na solucao atual */
	virtual Problema* mutacao(int) = 0;

	/* Devolve o valor da solucao */
	virtual double getFitness() const = 0;
	virtual double getFitnessMaximize() const = 0;	// Problemas de Maximizacao
	virtual double getFitnessMinimize() const = 0;	// Problemas de Minimizacao

private:

	virtual bool calcFitness(bool esc) = 0;		// Calcula o makespan


	friend bool fnequal1(Problema*, Problema*);	// Comparacao profunda
	friend bool fnequal2(Problema*, Problema*);	// Comparacao superficial
	friend bool fncomp1(Problema*, Problema*);	// Comparacao profunda
	friend bool fncomp2(Problema*, Problema*);	// Comparacao superficial
};

#endif
