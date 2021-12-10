#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/util/XMLString.hpp>

using namespace xercesc;
using namespace std;

#ifndef _Control_
#define _Control_

#include "Ateams.hpp"
#include "Problem.hpp"
#include "Heuristic.hpp"

#include "HeuristicTabu.hpp"
#include "HeuristicGenetic.hpp"
#include "HeuristicAnnealing.hpp"

extern volatile bool TERMINATE;

class Control: public DefaultHandler {
private:
	/* Instancia do controle */
	static Control *instance;

	static list<HeuristicListener*> *execAlgs;					// Algoritmos executados ate o momento
	static list<list<HeuristicListener*>::iterator> *actAlgs;	// Algoritmos em execucao no momento
	static int actThreads;											// Threads em execucao no momento

	/* Funcao que executa em multiplas threads e retorna o numero de solucoes inseridas */
	static void* pthrExec(void *obj);

	/* Funcao que cotrola o tempo de execucao */
	static void* pthrTime(void *obj);

	/* Funcao que controla a tela de informacoes */
	static void* pthrAnimation(void *in);

	static void display();                                      		//Desenha a tela
	static void reshape(int, int);                             	 		//Redesenha a tela
	static void drawstr(GLfloat, GLfloat, GLvoid*, const char*, ...); 	//Desenha uma string na tela

	static int window;

public:
	static int *argc;
	static char **argv;

	static Control* getInstance(int argc, char **argv);
	static void terminate();

	/* Retorna a soma de fitness de uma populacao */
	static double sumFitnessMaximize(set<Problem*, bool (*)(Problem*, Problem*)> *probs, int n);
	static double sumFitnessMaximize(vector<Problem*> *probs, int n);
	static double sumFitnessMinimize(set<Problem*, bool (*)(Problem*, Problem*)> *probs, int n);
	static double sumFitnessMinimize(vector<Problem*> *probs, int n);

	/* Seleciona um individuo da lista aleatoriamente, mas diretamente proporcional a sua qualidade */
	static set<Problem*, bool (*)(Problem*, Problem*)>::iterator selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *probs, double fitTotal);
	static vector<Problem*>::iterator selectRouletteWheel(vector<Problem*> *probs, double fitTotal);
	static Heuristic* selectRouletteWheel(vector<Heuristic*> *heuristc, unsigned int probTotal);

	/* Seleciona um individuo aleatoriamente */
	static vector<Problem*>::iterator selectRandom(vector<Problem*> *probs);

	static list<Problem*>::iterator findSol(list<Problem*> *vect, Problem *p);

private:

	char inputParameters[64];
	char inputDataFile[64];
	char outputResultFile[64];
	char outputLogFile[64];

	set<Problem*, bool (*)(Problem*, Problem*)> *pop; // Populacao principal
	vector<Heuristic*> *algs;							// Algoritmos disponiveis

	int numThreads;									// Numero de threads que podem rodar ao mesmo tempo
	int makespanBest;								// Melhor makespan conhecido
	int populationSize, iterAteams, tentAteams, maxTime;	// Tamanho da populacao, numero de iteracoes, tentativas sem melhora e tempo maximo de execucao
	int comparatorMode;								// Criterio de unicidade da populacao adotado

	bool printFullSolution;				// Imprime melhor solucao
	bool activeListener;				// Informa se as heuristicas serao acompanhadas por um listener

	time_t time1, time2;				// Medidor do tempo inicial e final
	int iterMelhora = 0;				// Ultima iteracao em que houve melhora
	int execThreads = 0;				// Threads executadas

	Control();

	~Control();

	/* Parser do arquivo XML de configuracoes */
	void startElement(const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname, const Attributes &attrs);

	/* Seleciona um dos algoritmos implementados para executar */
	int exec(int eID);

	/* Adiciona um novo conjunto de solucoes a populacao corrente */
	pair<int, int>* addSol(vector<Problem*> *news);

	/* Gera uma populacao inicial aleatoria com 'populationSize' elementos */
	void geraPop(list<Problem*> *popInicial);

	/* Leitura dos parametros passados por linha de comando */
	void readCMDParameters();

	/* Le parametros adicionais passados por linha de comando (Sobrepujam as lidas no arquivo de configuracao) */
	void readAdditionalCMDParameters();

	bool setParameter(const char *parameter, const char *value);

	void setPrintFullSolution(bool fullPrint);
	void setGraphicStatusInfoScreen(bool statusInfoScreen);

	/* Retorna a posicao em que o parametro esta em argv, ou -1 se nao existir */
	int findPosArgv(char **in, int num, char *key);

public:

	/* Adiciona uma heuristica ao conjunto de algoritmos disponiveis */
	void addHeuristic(Heuristic *alg);

	list<Problem*>* getPop();		// Retorna a populacao da memoria principal
	Problem* getSol(int n);		// Retorna a melhor solucao da memoria principal
	void getInfo(ExecInfo *info);	// Retorna algumas informacoes da ultima execucao

	/* Comeca a execucao do Ateams utilizando os algoritmos disponiveis */
	Problem* start(list<Problem*> *popInicial);

	void printSolution(Problem*);

	char* getInputDataFile() {
		return inputDataFile;
	}

	char* getInputParameters() {
		return inputParameters;
	}

	char* getOutputLogFile() {
		return outputLogFile;
	}

	char* getOutputResultFile() {
		return outputResultFile;
	}
};

inline bool cmpAlg(Heuristic *h1, Heuristic *h2) {
	return h1->choiceProbability < h2->choiceProbability;
}

#endif
