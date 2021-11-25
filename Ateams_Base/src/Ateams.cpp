#include "Ateams.hpp"

#include "Problema.hpp"

#include "Controle.hpp"

using namespace std;

#ifdef _WIN32
#define PARAMETROS "..\\Ateams_Base\\parametros\\DEFAULT.xml"
#else
#define PARAMETROS "../Ateams_Base/parametros/DEFAULT.xml"
#endif

volatile bool PARAR = false;

int main(int argc, char *argv[])
{
	/* Interrompe o programa ao se pessionar 'ctrl-c' */
	signal(SIGINT, Interrompe);

	srand(unsigned(time(NULL)));

	char dados[64];
	char resultado[64];
	char log[64];

	FILE *fdados;

	char *fXmlParametros = NULL;

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
		fXmlParametros = argv[p];
	}
	else
	{
		fXmlParametros = (char*)malloc((strlen(PARAMETROS) * sizeof(char)) + 1);
		strcpy(fXmlParametros, PARAMETROS);
	}

	if((p = findPosArgv(argv, argc, (char*)"-r")) != -1)
	{
		strcpy(resultado, argv[p]);

		printf("Resultado: '%s'\n", resultado);
	}
	else
	{
		mkdir("resultados");

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

	fclose(fdados);

	cout << endl;

	Controle::argc = &argc;
	Controle::argv = argv;

	/* Adiciona as heuristicas selecionadas */
	Controle* ctrl = Controle::getInstance(fXmlParametros);

	/* Le parametros adicionais passados por linha de comando (Sobrepujam as lidas no arquivo de configuracao) */
	ctrl->commandLineParameters();

	if(findPosArgv(argv, argc, (char*)"-g") == -1)
	{
		ctrl->statusInfoScreen(false);
	}
	else
	{
		ctrl->statusInfoScreen(true);
	}

	/* Le memoria prinipal do disco, se especificado */
	list<Problema*>* popInicial = *log == '\0' ? NULL : Problema::lePopulacao(log);

	/* Inicia a execucao */
	Problema* best = ctrl->start(popInicial);

	if(popInicial != NULL)
	{
		popInicial->clear();
		delete popInicial;
	}

	list<Problema*>* pop = ctrl->getPop();
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

	ExecInfo info;
	ctrl->getInfo(&info);

	/* Escreve solucao em arquivo no disco */
	Problema::escreveResultado(dados, fXmlParametros, &info, resultado);

	cout << endl << endl << "Solução:" << endl << endl;

	if(findPosArgv(argv, argc, (char*)"-d") != -1)
		best->imprimir(true);
	else
		best->imprimir(false);

	delete best;

	Controle::terminate();

	Problema::desalocaMemoria();

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
