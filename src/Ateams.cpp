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

volatile bool PARAR = false;

int main(int argc, char *argv[])
{
	/* Interrompe o programa ao se pessionar 'ctrl-c' */
	signal(SIGINT, Interrompe);

	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);

	srand(unsigned(time(NULL)));

	FILE *fdados;
	FILE *fparametros;
	FILE *fresultados;
	FILE *flog;

	ParametrosATEAMS *pATEAMS;
	vector<ParametrosHeuristicas> *pHEURISTICAS;

	pATEAMS = (ParametrosATEAMS*)malloc(sizeof(ParametrosATEAMS));
	pHEURISTICAS = new vector<ParametrosHeuristicas>;

	char dados[32];
	int p = -1;

	/* Leitura dos parametros passados por linha de comando */
	if((p = findPosArgv(argv, argc, (char*)"-i")) != -1)
	{
		if((fdados = fopen(argv[p], "r")) == NULL)
		{
			printf("\nArquivo \"%s\" não encontrado.\n\n", argv[p]);
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

	if((p = findPosArgv(argv, argc, (char*)"-p")) != -1)
	{
		if((fparametros = fopen(argv[p], "r")) == NULL)
		{
			printf("Arquivo \"%s\" não encontrado.\n\n", argv[p]);
			exit(1);
		}
		else
		{
			printf("Parâmetros: '%s'\n", argv[p]);
		}
	}
	else
	{
		fparametros = fopen(PARAMETROS, "r");
		printf("Parâmetros: '%s'\n", PARAMETROS);
	}

	char resultado[32] = {"resultados/"};
	if((p = findPosArgv(argv, argc, (char*)"-r")) != -1)
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

	if((p = findPosArgv(argv, argc, (char*)"-l")) != -1)
	{
		if((flog = fopen(argv[p], "r+")) == NULL)
		{
			flog = fopen(argv[p], "w+");
		}
		else
		{
			printf("Log: '%s'\n", argv[p]);
		}
	}
	else
	{
		flog = NULL;
	}

	/* Leitura dos dados passados por arquivos */
	Problema::leProblema(fdados);
	Problema::leParametros(fparametros, pATEAMS, pHEURISTICAS);

	fclose(fdados);
	fclose(fparametros);

	/* Le parametros adicionais passados por linha de comando */
	Problema::leArgumentos(argv, argc, pATEAMS);

	cout << endl;

	/* Adiciona as heuristicas selecionadas */
	Controle* ctr = new Controle(pATEAMS);
	for(int i = 0; i < (int)pHEURISTICAS->size(); i++)
	{
		if(pHEURISTICAS->at(i).alg == SA)
			ctr->addHeuristic(new Annealing(pHEURISTICAS->at(i).algName, pHEURISTICAS->at(i)));
		if(pHEURISTICAS->at(i).alg == AG)
			ctr->addHeuristic(new Genetico(pHEURISTICAS->at(i).algName, pHEURISTICAS->at(i)));
		if(pHEURISTICAS->at(i).alg == BT)
			ctr->addHeuristic(new Tabu(pHEURISTICAS->at(i).algName, pHEURISTICAS->at(i)));
	}

	/* Le memoria prinipal do disco, se especificado */
	list<Problema*>* popInicial = Problema::lePopulacao(flog);
	fseek(flog, 0, SEEK_SET);

	/* Inicia a execucao */
	Problema* best = ctr->start(popInicial);

	list<Problema*>* pop = ctr->getPop();
	list<Problema*>::const_iterator iter1, iter2;

	/* Testa a memoria principal por solucoes repetidas */
	for(iter1 = pop->begin(); iter1 != pop->end(); iter1++)
		for(iter2 = iter1; iter2 != pop->end(); iter2++)
			if((iter1 != iter2) && (fnequal1(*iter1, *iter2) || fncomp1(*iter2, *iter1)))
				cout << endl << "Memória Principal Incorreta!!!" << endl;

	/* Esreve memoria prncipal no disco */
	Problema::escrevePopulacao(flog, pop);
	fclose(flog);

	delete pop;

	cout << endl << endl << "Pior Solução: " << Problema::worst << endl;
	cout << endl << "Melhor Solução: " << Problema::best << endl << endl;

	/* Escreve solucao em arquivo no disco */
	gettimeofday(&tv2, NULL);
	fresultados = fopen(resultado, "a");
	Problema::imprimeResultado(tv1, tv2, fresultados, (int)best->getFitnessMinimize());
	fclose(fresultados);

	cout << endl << "Solução: " << endl << endl;
	best->imprimir(false);

	delete ctr;

	Problema::desalocaMemoria();

	free(pATEAMS);
	delete pHEURISTICAS;

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

/* Retorna a posicao em que o parametro esta em argv, ou -1 se nao existir */
int findPosArgv(char **in, int num, char *key)
{
	for(int i = 0; i < num; i++)
	{
		if(!strcmp(in[i], key))
			return i+1;
	}
	return -1;
}

float findPar(string& in, char *key)
{
	size_t pos = findPosPar(in, key);
	float ret = -1;
	char str[16] = "###############";

	if((int)pos != -1)
	{
		in.copy(str, 15, pos);
		sscanf(str, "%f", &ret);
	}
	return ret;
}

size_t findPosPar(string& in, char *key)
{
	size_t pos = in.find(key);

	if(pos != string::npos)
		return in.find("=", pos) + 1;
	else
		return (size_t)-1;
}

#ifdef _WIN32

#include <time.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

struct timezone
{
	int  tz_minuteswest; /* minutes W of Greenwich */
	int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;

	if (NULL != tv)
	{
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		/*converting file time to unix epoch*/
		tmpres /= 10;  /*convert into microseconds*/
		tmpres -= DELTA_EPOCH_IN_MICROSECS;
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}

	if (NULL != tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}
	return 0;
}

#endif
