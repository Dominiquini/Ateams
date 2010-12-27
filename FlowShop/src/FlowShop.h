#include "../../Ateams_Base/src/Problema.h"

#define INV_FITNESS 1000000
#define MAX_PERMUTACOES 10000

using namespace std;

#ifndef _FlowShop_
#define _FlowShop_

class Solucao_FlowShop : public Solucao
{
	short int *esc;			// Solucao
	short int ***escalon;	// Escalonamento nas maquinas - Grafico de Gant

	friend class Problema;
	friend class FlowShop;

	friend bool fnequal1(Problema*, Problema*);	// Comparacao profunda
	friend bool fnequal2(Problema*, Problema*);	// Comparacao superficial
	friend bool fncomp1(Problema*, Problema*);	// Comparacao profunda
	friend bool fncomp2(Problema*, Problema*);	// Comparacao superficial
};

class InfoTabu_FlowShop : public InfoTabu
{
private:

	short int A, B;

public:
	InfoTabu_FlowShop(int xA, int xB)
	{
		A = xA;
		B = xB;
	}

	// Verifica se 't1' eh igual a 't2'
	bool operator == (InfoTabu& movTabu)
	{
		InfoTabu_FlowShop* t = dynamic_cast<InfoTabu_FlowShop *>(&movTabu);

		if((A == t->A && B == t->B) || (A == t->B && B == t->A))
			return true;
		else
			return false;
	}
};

class FlowShop : public Problema
{
private:

	bool calcFitness(bool esc);		// Calcula o makespan

	Solucao_FlowShop sol;			// Representacao interna da solucao

public:

	static char name[128];			// Nome do problema

	static int **time;				// Matriz de maquinas e de tempos
	static int njob, nmaq;			// Quantidade de jobs e de maquinas

	static int num_vizinhos;		// Numero de permutacoes possiveis


	FlowShop();											// Nova solucao aleatoria
	FlowShop(short int *prob);							// Copia de prob
	FlowShop(const Problema &prob);						// Copia de prob
	FlowShop(const Problema &prob, int pos1, int pos2);	// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	virtual ~FlowShop();

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
	vector<pair<Problema*, InfoTabu*>* >* buscaLocal();			// Todos os vizinhos
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

	Solucao_FlowShop& getSoluction()
	{
		return sol;
	}

	friend bool fnequal1(Problema*, Problema*);	// Comparacao profunda
	friend bool fnequal2(Problema*, Problema*);	// Comparacao superficial
	friend bool fncomp1(Problema*, Problema*);	// Comparacao profunda
	friend bool fncomp2(Problema*, Problema*);	// Comparacao superficial
};

void swap_vect(short int* p1, short int* p2, short int* f, int pos, int tam);

void* alocaMatriz(int, int, int, int);

void desalocaMatriz(int, void*, int, int);

bool ptcomp(pair<Problema*, InfoTabu*>*, pair<Problema*, InfoTabu*>*);

bool find(vector<Problema*> *vect, Problema *p);

#endif
