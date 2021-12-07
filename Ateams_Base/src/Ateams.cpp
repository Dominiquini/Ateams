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
	signal(SIGINT, terminate);

	srand(unsigned(time(NULL)));

	char inputDataFile[64];
	char outputResultFile[64];
	char outputLogFile[64];

	FILE *fdados;

	char *fXmlParametros = NULL;

	int p = -1;

	/* Leitura dos parametros passados por linha de comando */
	if((p = findPosArgv(argv, argc, (char*)"-i")) != -1)
	{
		if((fdados = fopen(argv[p], "r")) == NULL)
		{
			printf("\nData File \"%s\" Not Found!.\n\n", argv[p]);
			exit(1);
		}
		else
		{
			printf("\nData File: '%s'\n", argv[p]);
			strcpy(inputDataFile, argv[p]);
		}
	}
	else
	{
		printf("\n./Ateams -i <INPUT_FILE -p <INPUT_PARAMETERS -r <RESULT_FILE> -l <LOG_FILE>\n\n");

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

		printf("Log File: '%s'\n", argv[p]);
	}
	else
	{
		outputLogFile[0] = '\0';

		printf("~Log File: ---\n");
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
	list<Problema*>* popInicial = *outputLogFile == '\0' ? NULL : Problema::lePopulacao(outputLogFile);

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
				cout << endl << "Incorrect Main Memory!!!" << endl;

	/* Escreve memoria principal no disco */
	if(*outputLogFile != '\0')
		Problema::escrevePopulacao(outputLogFile, pop);

	delete pop;

	cout << endl << endl << "Worst Final Solution: " << Problema::worst << endl;
	cout << endl << "Best Final Solution: " << Problema::best << endl;

	ExecInfo info;
	ctrl->getInfo(&info);

	/* Escreve solucao em arquivo no disco */
	if(*outputResultFile != '\0')
		Problema::escreveResultado(inputDataFile, fXmlParametros, &info, outputResultFile);

	cout << endl << endl << "Solution:" << endl << endl;

	if(findPosArgv(argv, argc, (char*)"-d") != -1)
		best->imprimir(true);
	else
		best->imprimir(false);

	delete best;

	Controle::terminate();

	Problema::desalocaMemoria();

	cout << endl << endl << "Explored Solutions: " << Problema::totalNumInst << endl << endl;

	if(Problema::numInst != 0)
		cout << "Memory Leak! ( " << Problema::numInst << " )" << endl << endl;

	cout << endl;

	return 0;
}

int xRand()
{
	return xRand(0, RAND_MAX);
}

int xRand(int min, int max)
{
	random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<int> distr(min, max-1);

	return distr(gen);
}

void terminate(int signal)
{
	PARAR = true;
}
