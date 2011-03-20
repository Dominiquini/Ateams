#include "../../Ateams_Base/src/Problema.hpp"

#define INV_FITNESS 1000000
#define MAX_PERMUTACOES 10000

using namespace std;

#ifndef _KnapSack_
#define _KnapSack_

class Solucao_KnapSack : public Solucao
{
private:

	short int *ordemItens;		// Solucao
	short int limit;			// Ponto onde nao cabe mais itens

	friend class Problema;
	friend class KnapSack;

	friend bool fnequal1(Problema*, Problema*);	// Comparacao profunda
	friend bool fnequal2(Problema*, Problema*);	// Comparacao superficial
	friend bool fncomp1(Problema*, Problema*);	// Comparacao profunda
	friend bool fncomp2(Problema*, Problema*);	// Comparacao superficial
};

class InfoTabu_KnapSack : public InfoTabu
{
private:

	short int A, B;

public:

	InfoTabu_KnapSack(int xA, int xB)
	{
		A = xA;
		B = xB;
	}

	// Verifica se 't1' eh igual a 't2'
	bool operator == (InfoTabu& movTabu)
	{
		InfoTabu_KnapSack* t = dynamic_cast<InfoTabu_KnapSack *>(&movTabu);

		if((A == t->A && B == t->B) || (A == t->B && B == t->A))
			return true;
		else
			return false;
	}
};

class KnapSack : public Problema
{
private:

	bool calcFitness(bool esc);		// Calcula o makespan

	Solucao_KnapSack sol;			// Representacao interna da solucao

public:

	static char name[128];			// Nome do problema

	static double *values, **constraint, *limit;	// Valores, limitacoes da esquerda e direita
	static int nitens, ncontraint;					// Quantidade de itens e de limitacoes

	static int num_vizinhos;		// Numero de permutacoes possiveis


	KnapSack();												// Nova solucao aleatoria
	KnapSack(short int *prob);								// Copia de prob
	KnapSack(short int *prob, int limit);					// Copia de prob
	KnapSack(const Problema &prob);							// Copia de prob
	KnapSack(const Problema &prob, int pos1, int pos2);		// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	~KnapSack();


	void imprimir(bool esc);		// Imprime o escalonamento atual

	/* Retorna um novo vizinho aleatorio */
	Problema* vizinho();

	/* Retorna um conjunto de solucoes viaveis vizinhas da atual. Retorna 'n' novos indivíduos */
	vector<pair<Problema*, InfoTabu*>* >* buscaLocal();		// Todos os vizinhos
	vector<pair<Problema*, InfoTabu*>* >* buscaLocal(float);	// Uma parcela aleatoria

	/* Faz o crossover da solucao atual com a passada como parametro. Retorna dois novos individuos */
	pair<Problema*, Problema*>* crossOver(const Problema*, int, int);	// Dois pivos
	pair<Problema*, Problema*>* crossOver(const Problema*, int);		// Um pivo

	/* Devolve uma mutacao aleatoria na solucao atual */
	Problema* mutacao(int);

	/* Devolve o valor da solucao */
	double getFitness() const;
	double getFitnessMaximize() const;
	double getFitnessMinimize() const;

	Solucao_KnapSack& getSoluction()
	{
		return sol;
	}

	friend bool fnequal1(Problema*, Problema*);	// Comparacao profunda
	friend bool fnequal2(Problema*, Problema*);	// Comparacao superficial
	friend bool fncomp1(Problema*, Problema*);	// Comparacao profunda
	friend bool fncomp2(Problema*, Problema*);	// Comparacao superficial
};

void swap_vect(short int* p1, short int* p2, short int* f, int pos, int tam);

bool constraintVerify(int item, vector<double> &constraints);

bool ptcomp(pair<Problema*, InfoTabu*>*, pair<Problema*, InfoTabu*>*);

bool find(vector<Problema*> *vect, Problema *p);

#endif
