#include "Problema.h"
#include "Controle.h"
#include "Ateams.h"

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
void lerArgumentos(char **argv, int argc, ParametrosATEAMS *pATEAMS, ParametrosBT *pBT, ParametrosAG *pAG)
{
	int p = -1;

	if((p = locComPar(argv, argc, (char*)"--agUtilizado")) != -1)
		pATEAMS->agenteUtilizado = atof(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--iterAteams")) != -1)
		pATEAMS->iteracoesAteams = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--MaxTempo")) != -1)
		pATEAMS->maxTempo = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--polAceitacao")) != -1)
		pATEAMS->politicaAceitacao = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--polDestruicao")) != -1)
		pATEAMS->politicaDestruicao = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--tamPopulacao")) != -1)
		pATEAMS->tamanhoPopulacao = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--makespanBest")) != -1)
		pATEAMS->makespanBest = atoi(argv[p]);


	if((p = locComPar(argv, argc, (char*)"--iterAG")) != -1)
		pAG->numeroIteracoes = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--polLeituraAG")) != -1)
		pAG->politicaLeitura = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--probCrossover")) != -1)
		pAG->probabilidadeCrossover = atof(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--probMutacao")) != -1)
		pAG->probabilidadeMutacoes = atof(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--Selecao")) != -1)
		pAG->selecao = atoi(argv[p]);


	if((p = locComPar(argv, argc, (char*)"--iterBT")) != -1)
		pBT->numeroIteracoes = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--polLeituraBT")) != -1)
		pBT->politicaLeitura = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--tamListaBT")) != -1)
		pBT->tamanhoListaTabu = atoi(argv[p]);
}

void imprimeResultado (struct timeval tv1, struct timeval tv2, FILE *resultados, int bestMakespan)
{
	int s = (((tv2.tv_sec*1000)+(tv2.tv_usec/1000)) - ((tv1.tv_sec*1000)+(tv1.tv_usec/1000)))/1000;

	fprintf(resultados, "%d %8d\n", bestMakespan, s);
}


int main(int argc, char *argv[])
{
	/* Interrompe o programa ao se pessionar 'ctrl-c' */
	signal(SIGINT, Interrompe);

	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);

	/* Leitura dos parametros passados por linha de comando */
	FILE *fdados;
	FILE *fparametros;
	FILE *fresultados;

	ParametrosATEAMS *pATEAMS;
	ParametrosAG *pAG;
	ParametrosBT *pBT;

	pATEAMS = (ParametrosATEAMS*)malloc(sizeof(ParametrosATEAMS));
	pBT = (ParametrosBT*)malloc(sizeof(ParametrosBT));
	pAG = (ParametrosAG*)malloc(sizeof(ParametrosAG));

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
	Problema::leParametros(fparametros, pATEAMS, pBT, pAG);

	fclose(fdados);
	fclose(fparametros);

	lerArgumentos(argv, argc, pATEAMS, pBT, pAG);

	pATEAMS->maxTempo = pATEAMS->maxTempo <= 0 ? INT_MAX : pATEAMS->maxTempo;

	cout << endl;

	Controle* ctr = new Controle(pATEAMS, new Tabu(pBT));
	Problema* best = ctr->start();
	cout << endl << "Melhor Solução: " << best->makespan << endl << endl;

	gettimeofday(&tv2, NULL);
	imprimeResultado(tv1, tv2, fresultados, best->makespan);
	fclose(fresultados);

	delete ctr;
	cout << Problema::numInst << " : " << ctr->pop->size() << endl << endl;

	return 0;
}
