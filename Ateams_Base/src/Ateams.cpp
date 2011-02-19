#include "Ateams.h"

#include "Problema.h"

#include "Controle.h"

#include "Tabu.h"
#include "Genetico.h"
#include "Annealing.h"

using namespace std;

#ifdef _WIN32
#define PARAMETROS "parametros\\DEFAULT.param"
#else
#define PARAMETROS "parametros/DEFAULT.param"
#endif

volatile bool PARAR = false;

int main(int argc, char *argv[])
{
	/* Interrompe o programa ao se pessionar 'ctrl-c' */
	signal(SIGINT, Interrompe);

	srand(unsigned(time(NULL)));

	char dados[32];
	char parametros[32];
	char resultado[32];
	char log[32];

	FILE *fdados;
	FILE *fparametros;

	ParametrosATEAMS *pATEAMS;
	vector<ParametrosHeuristicas> *pHEURISTICAS;

	pATEAMS = (ParametrosATEAMS*)malloc(sizeof(ParametrosATEAMS));
	pHEURISTICAS = new vector<ParametrosHeuristicas>;

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
		printf("\n./Ateams -i <ARQUIVO_ENTRADA -p <ARQUIVO_PARÂMETROS -r <ARQUIVO_RESULTADOS> -l <ARQUIVO_LOG>\n\n");

		return -1;
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
			strcpy(parametros, argv[p]);
		}
	}
	else
	{
		fparametros = fopen(PARAMETROS, "r");
		printf("Parâmetros: '%s'\n", PARAMETROS);
		strcpy(parametros, PARAMETROS);
	}

	if((p = findPosArgv(argv, argc, (char*)"-r")) != -1)
	{
		strcpy(resultado, argv[p]);

		printf("Resultado: '%s'\n", resultado);
	}
	else
	{
		mkdir("resultados", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

		string arq(dados);
		char *c = NULL;

#ifdef _WIN32
		strcpy(resultado, "resultados\\");

		size_t pos = arq.rfind("\\");
		if(pos != string::npos)
			c = &dados[pos+2];
		else
			c = dados;

#else
		strcpy(resultado, "resultados/");

		size_t pos = arq.rfind("/");
		if(pos != string::npos)
			c = &dados[pos+1];
		else
			c = dados;

#endif

		strcat(resultado, c);

		resultado[strlen(resultado) - 3] = '\0';
		strcat(resultado, "res");

		printf("Resultado: '%s'\n", resultado);
	}

	if((p = findPosArgv(argv, argc, (char*)"-l")) != -1)
	{
		strcpy(log, argv[p]);

		printf("Log: '%s'\n", argv[p]);
	}
	else
	{
		log[0] = '\0';

		printf("~Log: ---");
	}

	/* Leitura dos dados passados por arquivos */
	Problema::leProblema(fdados);
	Problema::leParametros(fparametros, pATEAMS, pHEURISTICAS);

	fclose(fdados);
	fclose(fparametros);

	/* Le parametros adicionais passados por linha de comando (Sobrepujam as lidas no arquivo de configuracao) */
	Problema::leArgumentos(argv, argc, pATEAMS);

	cout << endl;

	/* Adiciona as heuristicas selecionadas */
	Controle* ctr = NULL;
	if(findPosArgv(argv, argc, (char*)"-g") == -1)
		ctr = new Controle(pATEAMS, false);
	else
		ctr = new Controle(pATEAMS, true);

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
	list<Problema*>* popInicial = *log == '\0' ? NULL : Problema::lePopulacao(log);

	/* Inicia a execucao */
	Problema* best = ctr->start(popInicial);

	if(popInicial != NULL)
	{
		popInicial->clear();
		delete popInicial;
	}

	list<Problema*>* pop = ctr->getPop();
	list<Problema*>::const_iterator iter1, iter2;

	/* Testa a memoria principal por solucoes repetidas ou fora de ordem */
	for(iter1 = pop->begin(); iter1 != pop->end(); iter1++)
		for(iter2 = iter1; iter2 != pop->end(); iter2++)
			if((iter1 != iter2) && (fnequal1(*iter1, *iter2) || fncomp1(*iter2, *iter1)))
				cout << endl << "Memória Principal Incorreta!!!" << endl;

	/* Escreve memoria principal no disco */
	if(*log != '\0')
		Problema::escrevePopulacao(log, pop);

	delete pop;

	cout << endl << endl << "Pior Solução Final: " << Problema::worst << endl;
	cout << endl << "Melhor Solução Final: " << Problema::best << endl;

	execInfo info;
	ctr->getInfo(&info);

	/* Escreve solucao em arquivo no disco */
	Problema::escreveResultado(dados, parametros, &info, resultado);

	cout << endl << endl << "Solução:" << endl << endl;

	if(findPosArgv(argv, argc, (char*)"-d") != -1)
		best->imprimir(true);
	else
		best->imprimir(false);

	delete best;

	delete ctr;

	Problema::desalocaMemoria();

	free(pATEAMS);
	delete pHEURISTICAS;

	cout << endl << endl << "Soluções Exploradas: " << Problema::totalNumInst << endl << endl;

	if(Problema::numInst != 0)
		cout << "Vazamento De Memória! ( " << Problema::numInst << " )" << endl << endl;

	cout << endl;

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
	char str[16] = "###############";
	float ret = -1;

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
