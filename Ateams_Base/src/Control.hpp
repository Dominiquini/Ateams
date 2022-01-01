#include <GL/freeglut.h>

#include "Ateams.hpp"

using namespace this_thread;
using namespace std;

#ifndef _CONTROL_
#define _CONTROL_

#include "Problem.hpp"
#include "XMLParser.hpp"
#include "Heuristic.hpp"
#include "HeuristicTabu.hpp"
#include "HeuristicGenetic.hpp"
#include "HeuristicAnnealing.hpp"

#define THREAD_TIME_CONTROL_INTERVAL 1000

#define WINDOW_WIDTH 1250
#define WINDOW_HEIGHT 500
#define WINDOW_ANIMATION_UPDATE_INTERVAL 100

#define BUFFER_SIZE 4096

#define pthread_return(VALUE) pthread_exit((void*) VALUE) ; return (void*) VALUE ;

extern volatile TerminationInfo STATUS;

class ProgressBar;

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

class Control {
private:
	/* Instancia do controle */
	static Control *instance;

	static int runningThreads;											// Threads em execucao no momento

	ProgressBar *loadingProgressBar;
	ProgressBar *executionProgressBar;

	static char buffer[BUFFER_SIZE];

	/* Funcao que executa em multiplas threads e retorna o numero de solucoes inseridas */
	static void* pthrExecution(void *iteration);

	/* Funcao que cotrola o tempo de execucao */
	static void* pthrManagement(void *_);

	/* Funcao que controla a tela de informacoes */
	static void* pthrAnimation(void *_);

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

	static void printProgress(HeuristicListener *heuristic, pair<int, int> *insertion);

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

	bool printFullSolution;				// Imprime melhor solucao

	bool showTextOverview;				// Informa se as heuristicas serao visualizadas no prompt
	bool showGraphicalOverview;			// Informa se as heuristicas serao visualizadas graficamente

	time_t startTime, endTime;			// Medidor do tempo inicial e final
	int lastImprovedIteration = 0;		// Ultima iteracao em que houve melhora
	long executionCount = 0;			// Threads executadas
	uintptr_t swappedSolutions = 0;		// Numero de solucoes produzidas pelos algoritimos

	Control();

	~Control();

	/* Seleciona um dos algoritmos implementados para executar */
	int execute(int eID);

	/* Adiciona um novo conjunto de solucoes a populacao corrente */
	pair<int, int>* addSolutions(vector<Problem*> *news);

	/* Gera uma populacao inicial aleatoria com 'populationSize' elementos */
	void generatePopulation(list<Problem*> *popInicial);

	/* Retorna a posicao em que o parametro esta em argv, ou -1 se nao existir */
	int findPosArgv(char **in, int num, char *key);

	/* Leitura dos parametros passados por linha de comando */
	void readMainCMDParameters();

	/* Le parametros adicionais passados por linha de comando (Sobrepujam as lidas no arquivo de configuracao) */
	void readAdditionalCMDParameters();

	void setPrintFullSolution(bool fullPrint);

	void setCMDStatusInfoScreen(bool showCMDOverview);
	void setGraphicStatusInfoScreen(bool showGraphicalOverview);

public:

	void init();

	void finish();

	void run();									// Comeca a execucao do Ateams utilizando os algoritmos disponiveis

	list<Problem*>* getSolutions();				// Retorna a populacao da memoria principal
	Problem* getSolution(int n);				// Retorna a solucao n da memoria principal

	ExecutionInfo getExecutionInfo();			// Retorna algumas informacoes da ultima execucao

	void printSolution();

	void printExecution();

	bool setParameter(const char *parameter, const char *value);

	void addHeuristic(Heuristic *alg);

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
