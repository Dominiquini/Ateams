#include "Ateams.h"

using namespace std;

extern pthread_mutex_t mut_p;

#ifndef _PROBLEMA_
#define _PROBLEMA_

class Problema;

bool fncomp1(Problema*, Problema*);		//Se P1 for menor que P2
bool fncomp2(Problema*, Problema*); 	//Se P1 for menor que P2, considerando apenas o fitness
bool fnequal1(Problema*, Problema*);	//Se P1 for igual a P2
bool fnequal2(Problema*, Problema*);	//Se P1 for igual a P2, considerando apenas o fitness

class Problema
{
public:
	static double best;				// Melhor solucao do momento
	static double worst;			// Pior solucao do momento
	static int numInst;				// Quantidade de instancias criadas
	static long int totalNumInst;	// Quantidade total de problemas processados

	// Le arquivo de dados de entrada
	static void leProblema(FILE*);

	// Le parametros de entrada
	static void leParametros(FILE*, ParametrosATEAMS*, vector<ParametrosHeuristicas>*);
	static void leArgumentos(char**, int, ParametrosATEAMS*);
	static list<Problema*>* lePopulacao(char*);

	// Imprime em um arquivo os resultados da execucao
	static void escrevePopulacao(char*, list<Problema*>*);
	static void imprimeResultado(char*, char*, execInfo*, char*);

	// Desaloca as estruturas do problema
	static void desalocaMemoria();

	// Alocador generico
	static Problema* randSoluction();							// Nova solucao aleatoria
	static Problema* copySoluction(const Problema& prob);		// Copia de prob

	// Calcula a melhora (Resultado Positivo !!!) de newP em relacao a oldP
	static double compare(double oldP, double newP);
	static double compare(Problema& oldP, Problema& newP);


	executado exec;							// Algoritmos executados na solucao

	// Contrutor/Destrutor padrao: Incrementa ou decrementa um contador de instancias
	Problema() {pthread_mutex_lock(&mut_p); numInst++; totalNumInst++; pthread_mutex_unlock(&mut_p);}	// numInst++

	virtual ~Problema() {pthread_mutex_lock(&mut_p); numInst--; pthread_mutex_unlock(&mut_p);}			// numInst--


	virtual bool operator == (const Problema&) = 0;
	virtual bool operator != (const Problema&) = 0;
	virtual bool operator <= (const Problema&) = 0;
	virtual bool operator >= (const Problema&) = 0;
	virtual bool operator < (const Problema&) = 0;
	virtual bool operator > (const Problema&) = 0;

	virtual void imprimir(bool esc) = 0;	// Imprime o escalonamento

	/* Retorna um vizinho aleatorio da solucao atual */
	virtual Problema* vizinho() = 0;

	/* Retorna um conjunto de solucoes viaveis vizinhas da atual */
	virtual vector<pair<Problema*, movTabu*>* >* buscaLocal() = 0;		// Todos os vizinhos
	virtual vector<pair<Problema*, movTabu*>* >* buscaLocal(float) = 0;	// Uma parcela aleatoria

	/* Realiza um crossover com uma outra solucao */
	virtual pair<Problema*, Problema*>* crossOver(const Problema*, int) = 0;	// Dois pivos
	virtual pair<Problema*, Problema*>* crossOver(const Problema*) = 0;			// Um pivo

	/* Devolve uma mutacao aleatoria na solucao atual */
	virtual Problema* mutacao(int) = 0;

	/* Devolve o valor da solucao */
	virtual double getFitnessMaximize() const = 0;	// Problemas de Maximizacao
	virtual double getFitnessMinimize() const = 0;	// Problemas de Minimizacao

	/* Devolve a representacao interna da solucao */
	const soluction& getSoluction() const
	{
		return sol;
	}

protected:
	soluction sol;								// Representacao interna da solucao

private:
	virtual bool calcFitness(bool esc) = 0;		// Calcula o makespan


	friend bool fnequal1(Problema*, Problema*);	// Comparacao profunda
	friend bool fnequal2(Problema*, Problema*);	// Comparacao superficial
	friend bool fncomp1(Problema*, Problema*);	// Comparacao profunda
	friend bool fncomp2(Problema*, Problema*);	// Comparacao superficial
};

#endif
