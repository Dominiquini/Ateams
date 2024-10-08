#include "Ateams.hpp"

using namespace std;
using namespace pugi;
using namespace chrono;
using namespace this_thread;

#ifndef _CONTROL_
#define _CONTROL_

#include "ProgressBar.hpp"
#include "GraphicalOverview.hpp"

#include "Problem.hpp"

#include "Heuristic.hpp"
#include "HeuristicTabu.hpp"
#include "HeuristicGenetic.hpp"
#include "HeuristicAnnealing.hpp"

extern volatile TerminationInfo STATUS;

class ProgressBar;

class SimulatedAnnealing;
class TabuSearch;
class GeneticAlgorithm;

struct ExecutionInfo {

	milliseconds executionTime;
	unsigned int executionCount;

	double worstFitness;
	double bestFitness;

	unsigned long long exploredSolutions;
	unsigned int heuristicsSolutionsCount;
	milliseconds heuristicsExecutionTime;

	ExecutionInfo(int executionCount, milliseconds executionTime, int heuristicsSolutionsCount, milliseconds heuristicsExecutionTime) {
		this->executionTime = executionTime;
		this->executionCount = executionCount;

		this->heuristicsSolutionsCount = heuristicsSolutionsCount;
		this->heuristicsExecutionTime = heuristicsExecutionTime;

		this->worstFitness = Problem::worst;
		this->bestFitness = Problem::best;

		this->exploredSolutions = Problem::totalNumInst;
	}
};

struct PopulationImprovement {

	double oldBest, newBest;
	double oldWorst, newWorst;

	void setOldFitness(double best, double worst) {
		oldBest = best;
		oldWorst = worst;
	}

	void setNewFitness(double best, double worst) {
		newBest = best;
		newWorst = worst;
	}

	double getImprovementOnBestSolution() {
		return Problem::improvement(oldBest, newBest);
	}

	double getImprovementOnWorstSolution() {
		return Problem::improvement(oldWorst, newWorst);
	}
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

	void sanitizeParameters() {
		if (iterations < 0 || iterations > MAX_ITERATIONS) {
			iterations = MAX_ITERATIONS;
		}

		if (numThreads < 0 || numThreads > MAX_THREADS) {
			numThreads = MAX_THREADS;
		}

		if (populationSize < 0 || populationSize > MAX_POPULATION) {
			populationSize = MAX_POPULATION;
		}

		if (maxPopulationSize > MAX_POPULATION) {
			maxPopulationSize = MAX_POPULATION;
		}
	}
};

class Control {
private:

	static Control *instance;

	static int runningThreads;

	/* Funcao que executa diversos algotitmos (em thread separada) */
	static list<HeuristicExecutionInfo*> threadExecutions(queue<unsigned int> *ids);

	/* Funcao que executa um algitimo (em thread separada) */
	static HeuristicExecutionInfo* threadExecution(unsigned int executionId);

	/* Funcao que cotrola a execucao (em thread separada) */
	static TerminationInfo threadManagement();

public:

	static Control* getInstance(int argc, char **argv);
	static ExecutionInfo* terminate();

	static Heuristic* instantiateHeuristic(const char *name);

	static void printProgress(HeuristicExecutionInfo *heuristic);

private:

	int *argc;
	char **argv;

	char inputParameters[128];
	char inputDataFile[128];
	char outputResultFile[128];
	char populationFile[128];

	bool printFullSolution = false;				// Imprime melhor solucao por completo

	bool showTextOverview = false;				// Informa se as heuristicas serao visualizadas no prompt
	bool showTextFitnessOverview = false;		// Informa o melhor e pior fitness atual
	bool showTextImprovementOverview = false;	// Informa o melhor fitness antes e depois da execucao
	bool showTextThreadOverview = false;		// Informa se serao exibidas informacoes das threads no prompt
	bool showTextQueueOverview = false;			// Informa se serao exibidas informacoes da queue no prompt

	bool showGraphicalOverview = false;			// Informa se as heuristicas serao visualizadas graficamente

	bool singleExecutionPerThread = false;		// Cria uma thread para cada iteracao

	AteamsParameters parameters;

	list<Problem*> *savedPopulation = NULL;						// Populacao inicial do arquivo de log

	set<Problem*, bool (*)(Problem*, Problem*)> *solutions;		// Populacao principal

	int heuristicsProbabilitySum;								// Soma das probabilidades de todos os algoritimos
	vector<Heuristic*> *heuristics;								// Algoritmos disponiveis

	steady_clock::time_point startTime, endTime;				// Medidor do tempo inicial e final
	milliseconds executionTime = milliseconds::zero();			// Medidor da duracao de execucao
	int executionCount = 0;										// Threads executadas

	int iterationsWithoutImprovement = 0;						// Ultima iteracao em que houve melhora

	unsigned int heuristicsSolutionsCount = 0;							// Numero de solucoes produzidas pelos algoritimos
	milliseconds heuristicsTotalExecutionTime = milliseconds::zero();	// Tempo total gasto pelas heuristicas

	ProgressBar *loadingProgressBar;
	ProgressBar *executionProgressBar;

	GraphicalOverview *graphicalOverview;

	Control(int argc, char **argv);

	~Control();

	/* Seleciona um dos algoritmos implementados para executar */
	HeuristicExecutionInfo* execute(unsigned int executionId);

	/* Adiciona um novo conjunto de solucoes a populacao corrente */
	PopulationImprovement* insertNewSolutions(vector<Problem*> *newSolutions, bool autoTrim);

	/* Remove solucoes excedentes da populacao */
	void trimSolutions();

	/* Gera uma populacao inicial aleatoria com 'populationSize' elementos */
	void generateInitialPopulation();

	/* Leitura dos parametros passados por linha de comando */
	void readMainCMDParameters();

	/* Le parametros adicionais passados por linha de comando (Sobrepujam as lidas no arquivo de configuracao) */
	void readExtraCMDParameters();

	/* Le parametros do arquivo XML passado por linha de comando */
	void readXMLFileParameters();

	void setPrintFullSolution(bool fullPrint);

	void setGraphicStatusInfoScreen(bool showGraphicalOverview);
	void setCMDStatusInfoScreen(bool showText, bool showFitness, bool showImprovement, bool showThread, bool showQueue);

	void setSingleExecutionPerThread(bool singleExecutionPerThread);

	/* Retorna a posicao em que o parametro esta em argv, ou -1 se nao existir */
	int findCMDArgumentPosition(string key);

public:

	void start();

	void finish();

	void run();									// Comeca a execucao do Ateams utilizando os algoritmos disponiveis

	list<Problem*>* getSolutions();				// Retorna a populacao da memoria principal
	Problem* getSolution(int n);				// Retorna a solucao n da memoria principal

	ExecutionInfo* getExecutionInfo();			// Retorna algumas informacoes da ultima execucao

	void printPopulationInfo(string status);

	void printSolutionInfo();

	void printExecutionInfo();

	AteamsParameters getParameters();

	bool setParameter(const char *parameter, const char *value);

	bool insertHeuristic(Heuristic *alg, bool deleteIfNotInserted);

	bool removeHeuristic(Heuristic *alg, bool deleteIfRemoved);

	void clearHeuristics(bool deleteHeuristics);

	char* getInputDataFile() {
		return inputDataFile;
	}

	char* getInputParameters() {
		return inputParameters;
	}

	char* getOutputLogFile() {
		return populationFile;
	}

	char* getOutputResultFile() {
		return outputResultFile;
	}

	friend class GraphicalOverview;
};

#endif
