#include "Problema.h"
#include "Controle.h"
#include "Ateams.h"

#include "Tabu.h"
#include "Genetico.h"

using namespace std;

#define DADOS "dados/la01.prb"
#define PARAMETROS "parametros/default.param"

bool PARAR = false;

void Interrompe(int signum)
{
	PARAR = true;
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

	char dados[32];
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
			strcpy(dados, argv[p]);
		}
	}
	else
	{
		fdados = fopen(DADOS, "r");
		printf("\nDados: '%s'\n", DADOS);
		strcpy(dados, DADOS);
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
		char resultado[32] = {"resultados/"};
		strcat(resultado, strstr(dados, "dados/") + 6);
		resultado[strlen(resultado) - 3] = '\0';
		strcat(resultado, "res");

		fresultados = fopen(resultado, "a");
		printf("Resultado: '%s'\n", resultado);
	}

	Problema::leProblema(fdados);
	Problema::leParametros(fparametros, pATEAMS, pBT, pAG);

	fclose(fdados);
	fclose(fparametros);

	Problema::leArgumentos(argv, argc, pATEAMS, pBT, pAG);

	cout << endl;

	Controle* ctr = new Controle(pATEAMS);
//	ctr->addHeuristic(new Tabu(pBT));
	ctr->addHeuristic(new Genetico(pAG));

	Problema* best = ctr->start();

	cout << endl << endl << "Pior Solução: " << Problema::best << endl << endl;
	cout << endl << "Melhor Solução: " << best->getMakespan() << endl << endl;

	gettimeofday(&tv2, NULL);
	Problema::imprimeResultado(tv1, tv2, fresultados, best->getMakespan());
	fclose(fresultados);

	cout << endl << "Escalonamento: " << endl << endl;
	best->imprimir(false);

	delete ctr;

	cout << endl << endl << "Memória Alocada: " << Problema::numInst << endl << endl;

	Problema::desalocaMemoria();

	return 0;
}
