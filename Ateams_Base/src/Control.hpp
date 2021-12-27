#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLString.hpp>

#include "Ateams.hpp"

using namespace this_thread;
using namespace xercesc;
using namespace std;

#ifndef _CONTROL_
#define _CONTROL_

#include "Problem.hpp"

#include "Heuristic.hpp"
#include "HeuristicTabu.hpp"
#include "HeuristicGenetic.hpp"
#include "HeuristicAnnealing.hpp"

#define THREAD_TIME_CONTROL_INTERVAL 1000

#define WINDOW_WIDTH 1250
#define WINDOW_HEIGHT 500
#define WINDOW_ANIMATION_UPDATE_INTERVAL 100

extern volatile TerminationInfo STATUS;

class SimulatedAnnealing;
class TabuSearch;
class GeneticAlgorithm;

struct ExecutionInfo {
	double executionTime;
	int executionCount;
	long int exploredSolutions;
	double worstFitness;
	double bestFitness;
};

class Control: public DefaultHandler {

private:
	/* Instancia do controle */
	static Control *instance;

	static list<HeuristicListener*> *executedHeuristics;				// Algoritmos executados ate o momento
	static list<list<HeuristicListener*>::iterator> *runningHeuristics;	// Algoritmos em execucao no momento
	static int runningThreads;											// Threads em execucao no momento

	/* Funcao que executa em multiplas threads e retorna o numero de solucoes inseridas */
	static void* pthrExecution(void *obj);

	/* Funcao que cotrola o tempo de execucao */
	static void* pthrManagement(void *obj);

	/* Funcao que controla a tela de informacoes */
	static void* pthrAnimation(void *in);

	static void display();                                      		//Desenha a tela
	static void reshape(GLint, GLint);                             	 	//Redesenha a tela
	static void drawstr(GLfloat, GLfloat, GLvoid*, const char*, ...); 	//Desenha uma string na tela

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

	static list<Problem*>::iterator findSolution(list<Problem*> *vect, Problem *p);

private:

	char inputParameters[128];
	char inputDataFile[128];
	char outputResultFile[128];
	char outputLogFile[128];

	set<Problem*, bool (*)(Problem*, Problem*)> *solutions; 		// Populacao principal
	vector<Heuristic*> *heuristics;									// Algoritmos disponiveis

	int numThreads;						// Numero de threads que podem rodar ao mesmo tempo
	int bestKnownFitness;				// Melhor makespan conhecido
	int populationSize;					// Tamanho da populacao
	int iterations; 					// Numero de iteracoes
	int attemptsWithoutImprovement; 	// Tentativas sem melhora
	int maxExecutionTime;				// Tempo maximo de execucao
	int comparatorMode;					// Criterio de unicidade da populacao adotado

	bool printFullSolution;				// Imprime melhor solucao
	bool heuristicListener;				// Informa se as heuristicas serao acompanhadas por um listener

	time_t startTime, endTime;			// Medidor do tempo inicial e final
	int lastImprovedIteration = 0;		// Ultima iteracao em que houve melhora
	long executionCount = 0;			// Threads executadas
	uintptr_t swappedSolutions = 0;		// Numero de solucoes produzidas pelos algoritimos

	Control();

	~Control();

	/* Parser do arquivo XML de configuracoes */
	void startElement(const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname, const Attributes &attrs);

	/* Adiciona uma heuristica ao conjunto de algoritmos disponiveis */
	void addHeuristic(Heuristic *alg);

	/* Seleciona um dos algoritmos implementados para executar */
	int execute(int eID);

	/* Adiciona um novo conjunto de solucoes a populacao corrente */
	pair<int, int>* addSolutions(vector<Problem*> *news);

	/* Gera uma populacao inicial aleatoria com 'populationSize' elementos */
	void generatePopulation(list<Problem*> *popInicial);

	/* Leitura dos parametros passados por linha de comando */
	void readMainCMDParameters();

	/* Le parametros adicionais passados por linha de comando (Sobrepujam as lidas no arquivo de configuracao) */
	void readAdditionalCMDParameters();

	bool setParameter(const char *parameter, const char *value);

	void setPrintFullSolution(bool fullPrint);
	void setGraphicStatusInfoScreen(bool statusInfoScreen);

	/* Retorna a posicao em que o parametro esta em argv, ou -1 se nao existir */
	int findPosArgv(char **in, int num, char *key);

public:

	void init();

	void finish();

	void run();								// Comeca a execucao do Ateams utilizando os algoritmos disponiveis

	list<Problem*>* getSolutions();				// Retorna a populacao da memoria principal
	Problem* getSolution(int n);				// Retorna a solucao n da memoria principal
	void checkSolutions();						// Testa a memoria principal por solucoes repetidas ou fora de ordem

	ExecutionInfo getExecutionInfo();			// Retorna algumas informacoes da ultima execucao

	void printSolution(bool fullSolution);

	void printExecution();

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

#endif
