#include "../../Ateams_Base/src/Problem.hpp"

#define INV_FITNESS 1000000
#define MAX_PERMUTATIONS 10000

using namespace std;

#ifndef _TravellingSalesman_
#define _TravellingSalesman_

class Solution_TravellingSalesman: public Solution {
private:

	short int *ordemNodes;		// Ordem em que os itens serao alocados nas bolsas

	friend class Problem;
	friend class TravellingSalesman;

	friend bool fnequal1(Problem*, Problem*);	// Comparacao profunda
	friend bool fnequal2(Problem*, Problem*);	// Comparacao superficial
	friend bool fncomp1(Problem*, Problem*);	// Comparacao profunda
	friend bool fncomp2(Problem*, Problem*);	// Comparacao superficial
};

class InfoTabu_TravellingSalesman: public InfoTabu {
private:

	short int A, B;

public:
	InfoTabu_TravellingSalesman(int xA, int xB) {
		A = xA;
		B = xB;
	}

	// Verifica se 't1' eh igual a 't2'
	bool operator ==(InfoTabu &movTabu) {
		InfoTabu_TravellingSalesman *t = dynamic_cast<InfoTabu_TravellingSalesman*>(&movTabu);

		if ((A == t->A && B == t->B) || (A == t->B && B == t->A))
			return true;
		else
			return false;
	}
};

class TravellingSalesman: public Problem {
private:

	bool calcFitness();						// Calcula o makespan

	Solution_TravellingSalesman solution;	// Representacao interna da solucao

public:

	static char name[128];					// Nome do problema

	static double **edges;					// Peso das arestas que ligam os nos
	static int nnodes;						// Quantidade de nos

	static int neighbors;					// Numero de permutacoes possiveis

	TravellingSalesman();											// Nova solucao aleatoria
	TravellingSalesman(short int *prob);							// Copia de prob
	TravellingSalesman(const Problem &prob);						// Copia de prob
	TravellingSalesman(const Problem &prob, int pos1, int pos2);	// Copia de prob trocando 'pos1' com 'pos2' em 'maq'

	~TravellingSalesman();

	void print(bool esc);		// Imprime o escalonamento atual

	/* Retorna um novo vizinho aleatorio */
	Problem* neighbor();

	/* Retorna um conjunto de solucoes viaveis vizinhas da atual. Retorna 'n' novos indivíduos */
	vector<pair<Problem*, InfoTabu*>*>* localSearch();			// Todos os vizinhos
	vector<pair<Problem*, InfoTabu*>*>* localSearch(float);	// Uma parcela aleatoria

	/* Faz o crossover da solucao atual com a passada como parametro. Retorna dois novos individuos */
	pair<Problem*, Problem*>* crossOver(const Problem*, int, int);	// Dois pivos
	pair<Problem*, Problem*>* crossOver(const Problem*, int);		// Um pivo

	/* Devolve uma mutacao aleatoria na solucao atual */
	Problem* mutation(int);

	/* Devolve o valor da solucao */
	double getFitness() const;
	double getFitnessMaximize() const;
	double getFitnessMinimize() const;

	Solution_TravellingSalesman& getSoluction() {
		return solution;
	}

	friend bool fnequal1(Problem*, Problem*);	// Comparacao profunda
	friend bool fnequal2(Problem*, Problem*);	// Comparacao superficial
	friend bool fncomp1(Problem*, Problem*);	// Comparacao profunda
	friend bool fncomp2(Problem*, Problem*);	// Comparacao superficial
};

void swap_vect(short int *p1, short int *p2, short int *f, int pos, int tam);

bool ptcomp(pair<Problem*, InfoTabu*>*, pair<Problem*, InfoTabu*>*);

bool find(vector<Problem*> *vect, Problem *p);

#endif

/* Only include our version of getline() if the POSIX version isn't available. */

#if !(defined _POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200809L

#if !(defined SSIZE_MAX)
#define SSIZE_MAX (SIZE_MAX >> 1)
#endif

ssize_t getline(char **pline_buf, size_t *pn, FILE *fin) {
	const size_t INITALLOC = 16;
	const size_t ALLOCSTEP = 16;
	size_t num_read = 0;

	/* First check that none of our input pointers are NULL. */
	if ((NULL == pline_buf) || (NULL == pn) || (NULL == fin)) {
		errno = EINVAL;
		return -1;
	}

	/* If output buffer is NULL, then allocate a buffer. */
	if (NULL == *pline_buf) {
		*pline_buf = (char*) malloc(INITALLOC);
		if (NULL == *pline_buf) {
			/* Can't allocate memory. */
			return -1;
		} else {
			/* Note how big the buffer is at this time. */
			*pn = INITALLOC;
		}
	}

	/* Step through the file, pulling characters until either a newline or EOF. */

	{
		int c;
		while (EOF != (c = getc(fin))) {
			/* Note we read a character. */
			num_read++;

			/* Reallocate the buffer if we need more room */
			if (num_read >= *pn) {
				size_t n_realloc = *pn + ALLOCSTEP;
				char *tmp = (char*) realloc(*pline_buf, n_realloc + 1); /* +1 for the trailing NUL. */
				if (NULL != tmp) {
					/* Use the new buffer and note the new buffer size. */
					*pline_buf = tmp;
					*pn = n_realloc;
				} else {
					/* Exit with error and let the caller free the buffer. */
					return -1;
				}

				/* Test for overflow. */
				if (SSIZE_MAX < *pn) {
					errno = ERANGE;
					return -1;
				}
			}

			/* Add the character to the buffer. */
			(*pline_buf)[num_read - 1] = (char) c;

			/* Break from the loop if we hit the ending character. */
			if (c == '\n') {
				break;
			}
		}

		/* Note if we hit EOF. */
		if (EOF == c) {
			errno = 0;
			return -1;
		}
	}

	/* Terminate the string by suffixing NUL. */
	(*pline_buf)[num_read] = '\0';

	return (ssize_t) num_read;
}

#endif
