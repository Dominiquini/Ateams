#include "Ateams.h"

using namespace std;

extern pthread_mutex_t mut_p;

#ifndef _PROBLEMA_
#define _PROBLEMA_

class Problema;
class Solucao;

bool fncomp1(Problema*, Problema*);		//Se P1 for menor que P2
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
	static void leProblema(FILE*);

	// Le a especificacao do problema
	static list<Problema*>* lePopulacao(char*);

	// Imprime em um arquivo os resultados da execucao
	static void escrevePopulacao(char*, list<Problema*>*);
	static void escreveResultado(char*, char*, execInfo*, char*);

	// Aloca e Desaloca as estruturas do problema
	static void alocaMemoria();
	static void desalocaMemoria();

	// Alocador generico
	static Problema* randSoluction();							// Nova solucao aleatoria
	static Problema* copySoluction(const Problema& prob);		// Copia de prob

	// Calcula a melhora (Resultado Positivo !!!) de newP em relacao a oldP
	static double melhora(double oldP, double newP);
	static double melhora(Problema& oldP, Problema& newP);


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
	virtual vector<pair<Problema*, InfoTabu*>* >* buscaLocal() = 0;			// Todos os vizinhos
	virtual vector<pair<Problema*, InfoTabu*>* >* buscaLocal(float) = 0;	// Uma parcela aleatoria

	/* Realiza um crossover com uma outra solucao */
	virtual pair<Problema*, Problema*>* crossOver(const Problema*, int, int) = 0;	// Dois pivos
	virtual pair<Problema*, Problema*>* crossOver(const Problema*, int) = 0;		// Um pivo

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

public:

	// Le parametros de entrada passados por arquivos de configuracao
	static void leParametros(FILE *f, ParametrosATEAMS *pATEAMS, vector<ParametrosHeuristicas> *pHEURISTICAS)
	{
		string parametros, ateams_p, alg_p;
		vector<size_t> algs;
		char temp = '#';
		int numAlgs = 0;
		float par = -1;
		size_t found = 0;

		while((temp = fgetc(f)) != EOF)
			parametros.push_back(temp);

		while((found = parametros.find((char*)"##("), found) != string::npos)
		{
			parametros[found] = '\0';
			algs.push_back(found+1);
		}
		algs.push_back(found);

		numAlgs = algs.size()-1;

		ateams_p.assign(parametros, 0, algs[0]);

		par = findPar(ateams_p, (char*)"[iterAteams]");
		pATEAMS->iterAteams = (int)par;

		par = findPar(ateams_p, (char*)"[numThreads]");
		pATEAMS->numThreads = (int)par;

		par = findPar(ateams_p, (char*)"[tentAteams]");
		pATEAMS->tentAteams = (int)par;

		par = findPar(ateams_p, (char*)"[maxTempoAteams]");
		pATEAMS->maxTempoAteams = (int)par;

		par = findPar(ateams_p, (char*)"[tamPopAteams]");
		pATEAMS->tamPopAteams = (int)par;

		par = findPar(ateams_p, (char*)"[critUnicidade]");
		pATEAMS->critUnicidade = (int)par;

		par = findPar(ateams_p, (char*)"[makespanBest]");
		pATEAMS->makespanBest = (int)par;


		ParametrosHeuristicas pTEMP;
		for(int i = 0; i < numAlgs; i++)
		{
			alg_p.assign(parametros, algs[i], algs[i+1]-algs[i]);
			alg_p[6] = '\0';

			if(alg_p.find((char*)"#(SA)#") != string::npos)
			{
				pTEMP.alg = SA;

				found = alg_p.find((char*)"(name) = ", 7) + strlen((char*)"(name) = ");
				sscanf(alg_p.c_str()+found, "%s", pTEMP.algName);

				alg_p += 7;

				par = findPar(alg_p, (char*)"[probSA]");
				pTEMP.probSA = (int)par;

				par = findPar(alg_p, (char*)"[polEscolhaSA]");
				pTEMP.polEscolhaSA = (int)par;

				par = findPar(alg_p, (char*)"[maxIterSA]");
				pTEMP.maxIterSA = (int)par;

				par = findPar(alg_p, (char*)"[initTempSA]");
				pTEMP.initTempSA = (float)par;

				par = findPar(alg_p, (char*)"[finalTempSA]");
				pTEMP.finalTempSA = (float)par;

				par = findPar(alg_p, (char*)"[restauraSolSA]");
				pTEMP.restauraSolSA = (int)par;

				par = findPar(alg_p, (char*)"[alphaSA]");
				pTEMP.alphaSA = (float)par;

				par = findPar(alg_p, (char*)"[probElitismoSA]");
				pTEMP.probElitismoSA = (float)par;

				if(pTEMP.polEscolhaSA > pATEAMS->tamPopAteams)
					pTEMP.polEscolhaSA = pATEAMS->tamPopAteams;
			}
			else if(alg_p.find((char*)"#(AG)#") != string::npos)
			{
				pTEMP.alg = AG;

				found = alg_p.find((char*)"(name) = ", 7) + strlen((char*)"(name) = ");
				sscanf(alg_p.c_str()+found, "%s", pTEMP.algName);

				alg_p += 7;

				par = findPar(alg_p, (char*)"[probAG]");
				pTEMP.probAG = (int)par;

				par = findPar(alg_p, (char*)"[polEscolhaAG]");
				pTEMP.polEscolhaAG = (int)par;

				par = findPar(alg_p, (char*)"[iterAG]");
				pTEMP.iterAG = (int)par;

				par = findPar(alg_p, (char*)"[tamPopAG]");
				pTEMP.tamPopAG = (int)par;

				par = findPar(alg_p, (char*)"[tamParticaoAG]");
				pTEMP.tamParticaoAG = (int)par;

				par = findPar(alg_p, (char*)"[probCrossOverAG]");
				pTEMP.probCrossOverAG = (float)par;

				par = findPar(alg_p, (char*)"[powerCrossOverAG]");
				pTEMP.powerCrossOverAG = (float)par;

				par = findPar(alg_p, (char*)"[probMutacaoAG]");
				pTEMP.probMutacaoAG = (float)par;

				if(pTEMP.polEscolhaAG > pATEAMS->tamPopAteams)
					pTEMP.polEscolhaAG = pATEAMS->tamPopAteams;
			}
			else if(alg_p.find((char*)"#(BT)#") != string::npos)
			{
				pTEMP.alg = BT;

				found = alg_p.find((char*)"(name) = ", 7) + strlen((char*)"(name) = ");
				sscanf(alg_p.c_str()+found, "%s", pTEMP.algName);

				alg_p += 7;

				par = findPar(alg_p, (char*)"[probBT]");
				pTEMP.probBT = (int)par;

				par = findPar(alg_p, (char*)"[polEscolhaBT]");
				pTEMP.polEscolhaBT = (int)par;

				par = findPar(alg_p, (char*)"[iterBT]");
				pTEMP.iterBT = (int)par;

				par = findPar(alg_p, (char*)"[tentSemMelhoraBT]");
				pTEMP.tentSemMelhoraBT = (int)par;

				par = findPar(alg_p, (char*)"[tamListaBT]");
				pTEMP.tamListaBT = (int)par;

				par = findPar(alg_p, (char*)"[polExplorBT]");
				pTEMP.polExplorBT = (float)par;

				par = findPar(alg_p, (char*)"[funcAspiracaoBT]");
				pTEMP.funcAspiracaoBT = (float)par;

				par = findPar(alg_p, (char*)"[probElitismoBT]");
				pTEMP.probElitismoBT = (float)par;

				if(pTEMP.polEscolhaBT > pATEAMS->tamPopAteams)
					pTEMP.polEscolhaBT = pATEAMS->tamPopAteams;
			}
			else
			{
				printf("\n\nERRO NO ARQUIVO DE PARÂMETROS!!!\n\n");
				exit(0);
			}
			pHEURISTICAS->push_back(pTEMP);
		}
		return;
	}

	// Le parametros de entrada passados por linha de comando
	static void leArgumentos(char **argv, int argc, ParametrosATEAMS *pATEAMS)
	{
		int p = -1;

		if((p = findPosArgv(argv, argc, (char*)"--iterAteams")) != -1)
			pATEAMS->iterAteams = atoi(argv[p]);

		if((p = findPosArgv(argv, argc, (char*)"--numThreads")) != -1)
			pATEAMS->numThreads = atoi(argv[p]);

		if((p = findPosArgv(argv, argc, (char*)"--tentAteams")) != -1)
			pATEAMS->tentAteams = atoi(argv[p]);

		if((p = findPosArgv(argv, argc, (char*)"--maxTempoAteams")) != -1)
			pATEAMS->maxTempoAteams = atoi(argv[p]);

		if((p = findPosArgv(argv, argc, (char*)"--tamPopAteams")) != -1)
			pATEAMS->tamPopAteams = atoi(argv[p]);

		if((p = findPosArgv(argv, argc, (char*)"--critUnicidade")) != -1)
			pATEAMS->critUnicidade = atoi(argv[p]);

		if((p = findPosArgv(argv, argc, (char*)"--makespanBest")) != -1)
			pATEAMS->makespanBest = atoi(argv[p]);
	}
};

#endif
