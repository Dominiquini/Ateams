#include "Ateams.h"

#include "Problema.h"
#include "JobShop.h"

#include "Controle.h"

#include "Tabu.h"
#include "Genetico.h"
#include "Annealing.h"

using namespace std;

#define DADOS "dados/la01.prb"
#define PARAMETROS "parametros/default.param"

bool PARAR = false;

int main(int argc, char *argv[])
{
	/* Interrompe o programa ao se pessionar 'ctrl-c' */
	signal(SIGINT, Interrompe);

	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);

	srand(unsigned(time(NULL)));

	/* Leitura dos parametros passados por linha de comando */
	FILE *fdados;
	FILE *fparametros;
	FILE *fresultados;

	ParametrosATEAMS *pATEAMS;
	ParametrosAG *pAG;
	ParametrosBT *pBT;
	ParametrosSA *pSA;

	pATEAMS = (ParametrosATEAMS*)malloc(sizeof(ParametrosATEAMS));
	pBT = (ParametrosBT*)malloc(sizeof(ParametrosBT));
	pAG = (ParametrosAG*)malloc(sizeof(ParametrosAG));
	pSA = (ParametrosSA*)malloc(sizeof(ParametrosSA));

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

	char resultado[32] = {"resultados/"};
	if((p = locComPar(argv, argc, (char*)"-r")) != -1)
	{
		strcpy(resultado, argv[p]);

		printf("Resultado: '%s'\n", resultado);
	}
	else
	{
		strcat(resultado, strstr(dados, "dados/") + 6);
		resultado[strlen(resultado) - 3] = '\0';
		strcat(resultado, "res");

		printf("Resultado: '%s'\n", resultado);
	}

	Problema::leProblema(fdados);
	Problema::leParametros(fparametros, pATEAMS, pBT, pAG, pSA);

	fclose(fdados);
	fclose(fparametros);

	Problema::leArgumentos(argv, argc, pATEAMS, pBT, pAG, pSA);

	cout << endl;

	Controle* ctr = new Controle(pATEAMS);
	ctr->addHeuristic(new Tabu("BT", pBT));
	ctr->addHeuristic(new Genetico("AG", pAG));
	ctr->addHeuristic(new Annealing("SA", pSA));

	Problema* best = ctr->start();

	cout << endl << endl << "Pior Solução: " << Problema::worst << endl;
	cout << endl << "Melhor Solução: " << Problema::best << endl << endl;

	gettimeofday(&tv2, NULL);
	fresultados = fopen(resultado, "a");
	Problema::imprimeResultado(tv1, tv2, fresultados, best->getFitnessMinimize());
	fclose(fresultados);

	cout << endl << "Escalonamento: " << endl << endl;
	best->imprimir(false);

	delete ctr;

	Problema::desalocaMemoria();

	cout << endl << endl << "Memória Total Alocada: " << Problema::totalNumInst;
	cout << endl << endl << "Memória Ainda Alocada: " << Problema::numInst << endl << endl << endl;

	return 0;
}

void Interrompe(int signum)
{
	PARAR = true;
}

int xRand(int rand, int a, int b)
{
	return a + (int)((double)(b-a)*rand/(RAND_MAX+1.0));
}