#include "Ateams.hpp"

using namespace std;

#ifndef _CONTROL_
#define _CONTROL_

#include "GraphicalOverview.hpp"
#include "XMLParser.hpp"

#include "Problem.hpp"

#include "Heuristic.hpp"
#include "HeuristicTabu.hpp"
#include "HeuristicGenetic.hpp"
#include "HeuristicAnnealing.hpp"

#define COMMAND_LINE_PARAMETER_SUFFIX "--"

#define THREAD_MANAGEMENT_UPDATE_INTERVAL 1000

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
	long executionCount;

	double worstFitness;
	double bestFitness;

	unsigned long long exploredSolutions;
};

struct AteamsParameters {
	map<string, void*> keys = {
			{"iterations", &iterations},
			{"numThreads", &numThreads},
			{"populationSize", &populationSize},
			{"maxPopulationSize", &maxPopulationSize},
			{"attemptsWithoutImprovement", &attemptsWithoutImprovement},
			{"maxExecutionTime", &maxExecutionTime},
			{"maxSolutions", &maxSolutions},
			{"bestKnownFitness", &bestKnownFitness}
	};

	int iterations = 250; 					// Numero de iteracoes
	int numThreads = 8;						// Numero de threads que podem rodar ao mesmo tempo
	int populationSize = 500;				// Tamanho inicial da populacao
	int maxPopulationSize = -1;				// Tamanho maximo da populacao
	int attemptsWithoutImprovement = 100; 	// Tentativas sem melhora
	int maxExecutionTime = 3600;			// Tempo maximo de execucao
	int maxSolutions = -1;					// Numero maximo de solucoes para testar
	int bestKnownFitness = -1;				// Melhor makespan conhecido

	bool setParameter(const char *parameter, const char *value) {
		int read = EOF;

		for (map<string, void*>::const_iterator param = keys.begin(); param != keys.end(); param++) {
			if (strcasecmp(parameter, param->first.c_str()) == 0) {
				read = sscanf(value, "%d", (int*) param->second);
			}
		}

		return read != EOF;
	}
};

class Control {
private:

	static Control *instance;

	static int runningThreads;

	static char buffer[BUFFER_SIZE];

	/* Funcao que executa em multiplas threads e retorna o numero de solucoes inseridas */
	static void* pthrExecution(void *iteration);

	/* Funcao que cotrola o tempo de execucao */
	static void* pthrManagement(void *_);

public:
	static int *argc;
	static char **argv;

	static Control* getInstance(int argc, char **argv);
	static void terminate();

	static Heuristic* instantiateHeuristic(char* name);

	/* Retorna a soma de fitness de uma populacao */
	static double sumFitnessMaximize(set<Problem*, bool (*)(Problem*, Problem*)> *probs, int n);
	static double sumFitnessMaximize(vector<Problem*> *probs, int n);
	static double sumFitnessMinimize(set<Problem*, bool (*)(Problem*, Problem*)> *probs, int n);
	static double sumFitnessMinimize(vector<Problem*> *probs, int n);

	/* Seleciona um individuo da lista aleatoriamente, mas diretamente proporcional a sua qualidade */
	static set<Problem*>::iterator selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *probs, double fitTotal);
	static vector<Problem*>::iterator selectRouletteWheel(vector<Problem*> *probs, double fitTotal);
	static Heuristic* selectRouletteWheel(vector<Heuristic*> *heuristc, unsigned int probTotal);

	/* Seleciona um individuo aleatoriamente */
	static vector<Problem*>::iterator selectRandom(vector<Problem*> *probs);

	static list<Problem*>::iterator findSolution(list<Problem*> *vect, Problem *p);

	static void printProgress(HeuristicExecutionInfo *heuristic);

private:

	char inputParameters[128];
	char inputDataFile[128];
	char outputResultFile[128];
	char outputLogFile[128];

	bool printFullSolution;				// Imprime melhor solucao

	bool showTextOverview;				// Informa se as heuristicas serao visualizadas no prompt
	bool showGraphicalOverview;			// Informa se as heuristicas serao visualizadas graficamente

	AteamsParameters parameters;

	set<Problem*, bool (*)(Problem*, Problem*)> *solutions; 		// Populacao principal
	vector<Heuristic*> *heuristics;									// Algoritmos disponiveis

	time_t startTime, endTime;			// Medidor do tempo inicial e final
	int lastImprovedIteration = 0;		// Ultima iteracao em que houve melhora
	long executionCount = 0;			// Threads executadas
	uintptr_t newSolutionsCount = 0;		// Numero de solucoes produzidas pelos algoritimos

	ProgressBar *loadingProgressBar;
	ProgressBar *executionProgressBar;

	GraphicalOverview *graphicalOverview;

	Control();

	~Control();

	/* Seleciona um dos algoritmos implementados para executar */
	HeuristicExecutionInfo* execute(unsigned int executionId);

	/* Adiciona um novo conjunto de solucoes a populacao corrente */
	void insertNewSolutions(vector<Problem*> *newSolutions, bool autoTrim);

	/* Remove soluções excedentes da população */
	void trimSolutions();

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

	AteamsParameters getParameters();

	bool setParameter(const char *parameter, const char *value);

	void newHeuristic(Heuristic *alg);

	void deleteHeuristic(Heuristic *alg);

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
