#include <stdio.h>

#include <functional>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <list>
#include <set>

using namespace std;

#ifndef _PROBLEMA_
#define _PROBLEMA_


typedef struct ParametrosATEAMS {
  float agenteUtilizado;
  int tamanhoPopulacao;   /* > 0*/
  int iteracoesAteams;
  int maxTempo;
  int politicaAceitacao;
  int politicaDestruicao;
  int makespanBest;       /* Melhor makespan conhecido */
} ParametrosATEAMS;

typedef struct ParametrosAG {
  int selecao;
  int quantidadeLeituraMemoriaATEAMS;
  int tamanhoPopulacao;
  int numeroIteracoes;
  int politicaLeitura;
  float probabilidadeCrossover;
  float probabilidadeMutacoes;
} ParametrosAG;

typedef struct ParametrosBT {
  int politicaLeitura;
  int numeroIteracoes;
  int tamanhoListaTabu;
  int k;                /* valor que ira determinar o tamanho da lista tabu de acordo como o numero de iteracoes */
} ParametrosBT;


typedef struct movTabu
{
	int maq, A, B;
} mov;


class Problema;

bool fncomp(Problema*, Problema*);


class Problema
{
public:
	static int numInst;				// Quantidade de instancias criadas

	static char name[128];			// Nome do problema
	static int **maq, **time;		// Matriz de maquinas e de tempos
	static int njob, nmaq;			// QUantidade de jobs e de maquinas

	static ParametrosATEAMS *pATEAMS;
	static ParametrosAG *pAG;
	static ParametrosBT *pBT;

	static void leProblema(FILE*);	// Le arquivo de dados de entrada
	static void leParametros(FILE*);// Le arquivo de parametros de entrada


	mov movTabu;	// Movimento tabu que gerou a solucao. movTabu.maq = -1 se por outro meio

	int **esc;		// Solucao
	int makespan;	// Makespan da solucao
	int ***escalon;	// Escalonamento nas maquinas

	Problema();		// numInst++

	~Problema();	// numInst--

	virtual int calcMakespan() = 0;	// Calcula o makespan
	virtual void imprimir() = 0;	// Imprime o escalonamento

	/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual */
	virtual multiset<Problema*, bool(*)(Problema*, Problema*)>* buscaLocal() = 0;
};

#endif
