#include "../../Ateams_Base/src/Problema.h"

#define INV_FITNESS 1000000
#define MAX_PERMUTACOES 10000

using namespace std;

#ifndef _KnapSack_2_
#define _KnapSack_2_

class Solucao_KnapSack_2 : public Solucao
{
private:

	short int *itens;		// Solucao (Itens Escolhidos)

	friend class Problema;
	friend class KnapSack_2;

	friend bool fnequal1(Problema*, Problema*);	// Comparacao profunda
	friend bool fnequal2(Problema*, Problema*);	// Comparacao superficial
	friend bool fncomp1(Problema*, Problema*);	// Comparacao profunda
	friend bool fncomp2(Problema*, Problema*);	// Comparacao superficial
};

class InfoTabu_KnapSack_2 : public InfoTabu
{
private:

	short int A, B;

public:

	InfoTabu_KnapSack_2(int xA, int xB)
	{
		A = xA;
		B = xB;
	}

	// Verifica se 't1' eh igual a 't2'
	bool operator == (InfoTabu& movTabu)
	{
		InfoTabu_KnapSack_2* t = dynamic_cast<InfoTabu_KnapSack_2 *>(&movTabu);

		if((A == t->A && B == t->B) || (A == t->B && B == t->A))
			return true;
		else
			return false;
	}
};

class KnapSack_2 : public Problema
{
private:

	bool calcFitness(bool esc);		// Calcula o makespan

	Solucao_KnapSack_2 sol;			// Representacao interna da solucao

public:

	static char name[128];			// Nome do problema

	static double *values, **constraint, *limit;	// Valores, limitacoes da esquerda e direita
	static int nitens, ncontraint;					// Quantidade de itens e de limitacoes

	static int num_vizinhos;		// Numero de permutacoes possiveis


	KnapSack_2();												// Nova solucao aleatoria
	KnapSack_2(short int *prob);								// Copia de prob
	KnapSack_2(const Problema &prob);							// Copia de prob
	KnapSack_2(const Problema &prob, int pos1, int pos2);		// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	virtual ~KnapSack_2();

	bool operator == (const Problema&);
	bool operator != (const Problema&);
	bool operator <= (const Problema&);
	bool operator >= (const Problema&);
	bool operator < (const Problema&);
	bool operator > (const Problema&);

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

	Solucao_KnapSack_2& getSoluction()
	{
		return sol;
	}

	friend bool fnequal1(Problema*, Problema*);	// Comparacao profunda
	friend bool fnequal2(Problema*, Problema*);	// Comparacao superficial
	friend bool fncomp1(Problema*, Problema*);	// Comparacao profunda
	friend bool fncomp2(Problema*, Problema*);	// Comparacao superficial
};

void swap_vect(short int* p1, short int* p2, short int* filho, int posInicio, int posFim);

bool constraintVerify(int c, short int* itens);

bool ptcomp(pair<Problema*, InfoTabu*>*, pair<Problema*, InfoTabu*>*);

bool find(vector<Problema*> *vect, Problema *p);

#endif
