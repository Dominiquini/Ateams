#include <unistd.h>
#include <iostream>
#include <cstdint>
#include <string>
#include <cctype>

#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/util/XMLString.hpp>

using namespace xercesc;
using namespace std;

#ifndef _Controle_
#define _Controle_

#include "Ateams.hpp"
#include "Problema.hpp"
#include "Heuristica.hpp"

#include "Tabu.hpp"
#include "Genetico.hpp"
#include "Annealing.hpp"

extern volatile bool TERMINATE;

class Controle : public DefaultHandler
{
private:
	/* Instancia do controle */
	static Controle* instance;

	static list<Heuristica_Listener*>* execAlgs;					// Algoritmos executados ate o momento
	static list<list<Heuristica_Listener*>::iterator >* actAlgs;	// Algoritmos em execucao no momento
	static int actThreads;											// Threads em execucao no momento

	/* Funcao que executa em multiplas threads e retorna o numero de solucoes inseridas */
	static void* pthrExec(void *obj);

	/* Funcao que cotrola o tempo de execucao */
	static void* pthrTime(void *obj);

	/* Funcao que controla a tela de informacoes */
	static void* pthrAnimation(void* in);

	static void display();                                      		//Desenha a tela
	static void reshape(int, int);                             	 		//Redesenha a tela
	static void drawstr(GLfloat, GLfloat, GLvoid*, const char*, ...); 	//Desenha uma string na tela

	static int window;

public:
	static int* argc;
	static char** argv;

	static Controle* getInstance(char* xml);
	static void terminate();

	// Retorna a soma de fitness de uma populacao
	static double sumFitnessMaximize(set<Problema*, bool(*)(Problema*, Problema*)> *probs, int n);
	static double sumFitnessMaximize(vector<Problema*> *probs, int n);
	static double sumFitnessMinimize(set<Problema*, bool(*)(Problema*, Problema*)> *probs, int n);
	static double sumFitnessMinimize(vector<Problema*> *probs, int n);

	/* Seleciona um individuo da lista aleatoriamente, mas diretamente proporcional a sua qualidade */
	static set<Problema*, bool(*)(Problema*, Problema*)>::iterator selectRouletteWheel(set<Problema*, bool(*)(Problema*, Problema*)>* probs, double fitTotal);
	static vector<Problema*>::iterator selectRouletteWheel(vector<Problema*>* probs, double fitTotal);
	static Heuristica* selectRouletteWheel(vector<Heuristica*>* heuristc, unsigned int probTotal);

	/* Seleciona um individuo aleatoriamente */
	static vector<Problema*>::iterator selectRandom(vector<Problema*>* probs);

	static list<Problema*>::iterator findSol(list<Problema*> *vect, Problema *p);

private:

	set<Problema*, bool(*)(Problema*, Problema*)>* pop; // Populacao principal
	vector<Heuristica*>* algs;							// Algoritmos disponiveis

	int numThreads;									// Número de threads que podem rodar ao mesmo tempo
	int makespanBest;								// Melhor makespan conhecido
	int tamPop, iterAteams, tentAteams, maxTempo;	// Tamanho da populacao, numero de iteracoes, tentativas sem melhora e tempo maximo de execucao
	int critUnicidade;								// Criterio de unicidade da populacao adotado

	bool activeListener;				// Informa se as heuristicas serao acompanhadas por um listener

	time_t time1, time2;				// Medidor do tempo inicial e final
	int iterMelhora = 0;				// Ultima iteracao em que houve melhora
	int execThreads = 0;				// Threads executadas

	Controle();

	~Controle();

	/* Parser do arquivo XML de configuracoes */
	void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs);

	/* Seleciona um dos algoritmos implementados para executar */
	int exec(int eID);

	/* Adiciona um novo conjunto de solucoes a populacao corrente */
	pair<int, int>* addSol(vector<Problema*> *news);

	/* Gera uma populacao inicial aleatoria com 'tamPop' elementos */
	void geraPop(list<Problema*>* popInicial);

public:

	/* Adiciona uma heuristica ao conjunto de algoritmos disponiveis */
	void addHeuristic(Heuristica* alg);

	list<Problema*>* getPop();		// Retorna a populacao da memoria principal
	Problema* getSol(int n);		// Retorna a melhor solucao da memoria principal
	void getInfo(ExecInfo *info);	// Retorna algumas informacoes da ultima execucao

	bool setParameter(const char* parameter, const char* value);
	void statusInfoScreen(bool status);

	void commandLineParameters();

	/* Comeca a execucao do Ateams utilizando os algoritmos disponiveis */
	Problema* start(list<Problema*>* popInicial);
};

inline bool cmpAlg(Heuristica *h1, Heuristica *h2)
{
	return h1->prob < h2->prob;
}

/* Retorna a posicao em que o parametro esta em argv, ou -1 se nao existir */
inline int findPosArgv(char **in, int num, char *key)
{
	for(int i = 0; i < num; i++)
	{
		if(!strcmp(in[i], key))
			return i+1;
	}

	return -1;
}

/* Leitura dos parametros passados por linha de comando */
inline void readCMDParameters(int argc, char** argv, char* inputParameters, char* inputDataFile, char* outputResultFile, char* outputLogFile)
{
	int p = -1;

	if((p = findPosArgv(argv, argc, (char*)"-p")) != -1)
	{
		strcpy(inputParameters, argv[p]);

		printf("Parameters File: '%s'\n", inputParameters);
	}
	else
	{
		inputParameters[0] = '\0';

		printf("Parameters File Cannot Be Empty!\n");

		printf("\n./Ateams -i <<INPUT_FILE>> -p <<INPUT_PARAMETERS>> -r <RESULT_FILE> -l <LOG_FILE>\n\n");

		exit(1);
	}

	if((p = findPosArgv(argv, argc, (char*)"-i")) != -1)
	{
		strcpy(inputDataFile, argv[p]);

		printf("Data File: '%s'\n", inputDataFile);
	}
	else
	{
		inputDataFile[0] = '\0';

		printf("Data File Cannot Be Empty!\n");

		printf("\n./Ateams -i <<INPUT_FILE>> -p <<INPUT_PARAMETERS>> -r <RESULT_FILE> -l <LOG_FILE>\n\n");

		exit(1);
	}

	if((p = findPosArgv(argv, argc, (char*)"-r")) != -1)
	{
		strcpy(outputResultFile, argv[p]);

		printf("Result File: '%s'\n", outputResultFile);
	}
	else
	{
		outputResultFile[0] = '\0';

		printf("~Result File: ---\n");
	}

	if((p = findPosArgv(argv, argc, (char*)"-l")) != -1)
	{
		strcpy(outputLogFile, argv[p]);

		printf("Log File: '%s'\n", outputLogFile);
	}
	else
	{
		outputLogFile[0] = '\0';

		printf("~Log File: ---\n");
	}
}

inline string capitalize(string text) {

	for (unsigned long x = 0; x < text.length(); x++)
	{
		if (x == 0)
		{
			text[x] = toupper(text[x]);
		}
		else if (text[x - 1] == ' ')
		{
			text[x] = toupper(text[x]);
		}
	}

	return text;
}

#endif
