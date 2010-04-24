#include <signal.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include "Problema.h"
#include "Controle.h"

using namespace std;

#define PARAMETROS "parametros/default.param"

int PARAR = 0;

void Interrompe(int signum)
{
	PARAR = 1;
}

/* Retorna a posicao em que o parametro esta em argv, ou -1 se nao existir */
int locComPar(char **in, int num, char *key)
{
	for(int i = 0; i < num; i++)
	{
		if(!strcmp(in[i], key))
			return i+1;
	}
	return -1;
}

/* Le argumentos adicionais passados por linha de comando */
void lerArgumentos(char **argv, int argc)
{
	int p = -1;

	if((p = locComPar(argv, argc, (char*)"--agUtilizado")) != -1)
		Problema::pATEAMS->agenteUtilizado = atof(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--iterAteams")) != -1)
		Problema::pATEAMS->iteracoesAteams = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--MaxTempo")) != -1)
		Problema::pATEAMS->maxTempo = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--polAceitacao")) != -1)
		Problema::pATEAMS->politicaAceitacao = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--polDestruicao")) != -1)
		Problema::pATEAMS->politicaDestruicao = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--tamPopulacao")) != -1)
		Problema::pATEAMS->tamanhoPopulacao = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--makespanBest")) != -1)
		Problema::pATEAMS->makespanBest = atoi(argv[p]);


	if((p = locComPar(argv, argc, (char*)"--iterAG")) != -1)
		Problema::pAG->numeroIteracoes = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--polLeituraAG")) != -1)
		Problema::pAG->politicaLeitura = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--probCrossover")) != -1)
		Problema::pAG->probabilidadeCrossover = atof(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--probMutacao")) != -1)
		Problema::pAG->probabilidadeMutacoes = atof(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--Selecao")) != -1)
		Problema::pAG->selecao = atoi(argv[p]);


	if((p = locComPar(argv, argc, (char*)"--iterBT")) != -1)
		Problema::pBT->numeroIteracoes = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--polLeituraBT")) != -1)
		Problema::pBT->politicaLeitura = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--tamListaBT")) != -1)
		Problema::pBT->tamanhoListaTabu = atoi(argv[p]);
}

void imprimeResultado (struct timeval tv1, struct timeval tv2, FILE *resultados, int bestMakespan)
{
	int s = (((tv2.tv_sec*1000)+(tv2.tv_usec/1000)) - ((tv1.tv_sec*1000)+(tv1.tv_usec/1000)))/1000;

	fprintf(resultados, "%8d %8d\n", bestMakespan, s);
}


int main(int argc, char *argv[])
{
	/* Interrompe o programa ao se pessionar 'ctrl-c' */
	signal(SIGINT, Interrompe);

	struct timeval tv1, tv2;
	gettimeofday(&tv2, NULL);

	/* Leitura dos parametros passados por linha de comando */
	FILE *fdados;
	FILE *fparametros;
	FILE *fresultados;

	Problema::pATEAMS = (ParametrosATEAMS*)malloc(sizeof(ParametrosATEAMS));
	Problema::pBT = (ParametrosBT*)malloc(sizeof(ParametrosBT));
	Problema::pAG = (ParametrosAG*)malloc(sizeof(ParametrosAG));

	int p = -1;

	if((p = locComPar(argv, argc, (char*)"-i")) != -1)
	{
		if ((fdados = fopen(argv[p], "r")) == NULL)
		{
			printf("\nArquivo \"%s\" nao encontrado.\n\n", argv[p]);
			exit(1);
		}
		else
		{
			printf("\nDados: '%s'\n", argv[p]);
		}
	}
	else
	{
		printf("\nEscolha um arquivo de entrada: 'ateams -i arq.prb'.\n\n");
		exit(1);
	}

	if((p = locComPar(argv, argc, (char*)"-p")) != -1)
	{
		if ((fparametros = fopen(argv[p], "r")) == NULL)
		{
			printf("Arquivo \"%s\" nao encontrado.\n\n", argv[p]);
			exit(1);
		}
		else
		{
			printf("Parametros: '%s'\n", argv[p]);
		}
	}
	else
	{
		fparametros = fopen(PARAMETROS, "r");
		printf("Parametros: '%s'\n", PARAMETROS);
	}

	if((p = locComPar(argv, argc, (char*)"-r")) != -1)
	{
		if ((fresultados = fopen(argv[p], "a")) == NULL)
		{
			printf("Arquivo \"%s\" nao encontrado.\n\n", argv[p]);
			exit(1);
		}
		else
		{
			printf("Resultados: '%s'\n", argv[p]);
		}
	}
	else
	{
		p = locComPar(argv, argc, (char*)"-i");

		char resultado[32] = {"resultados/"};
		strcat(resultado, strstr(argv[p], "dados/") + 6);
		resultado[strlen(resultado) - 3] = '\0';
		strcat(resultado, "res");

		fresultados = fopen(resultado, "a");
		printf("Resultado: '%s'\n", resultado);
	}

	Problema::leProblema(fdados);
	Problema::leParametros(fparametros);

	fclose(fdados);
	fclose(fparametros);

	lerArgumentos(argv, argc);

	cout << endl;

	Controle* ctr = new Controle(Problema::pATEAMS, new Tabu(Problema::pBT));
	Problema* best = ctr->start();
	cout << endl << "Melhor Solução: " << best->makespan << endl << endl;

	gettimeofday(&tv2, NULL);
	imprimeResultado(tv1, tv2, fresultados, best->makespan);
	fclose(fresultados);

	delete ctr;
	cout << Problema::numInst << " : " << ctr->pop->size() << endl << endl;

	return 0;
}
