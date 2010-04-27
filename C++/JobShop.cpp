#include "JobShop.h"

using namespace std;

/* Static Members */

int Problema::numInst = 0;
double Problema::totalMakespan = 0;

char JobShop::name[128];
int **JobShop::maq = NULL, **JobShop::time = NULL;
int JobShop::njob = 0, JobShop::nmaq = 0;


Problema::Problema()
{
	numInst++;
}

Problema::~Problema()
{
	numInst--;

	if(sol.esc != NULL)
		desalocaMatriz(2, sol.esc, JobShop::nmaq, 0);

	if(sol.escalon != NULL)
		desalocaMatriz(3, sol.escalon, JobShop::nmaq, JobShop::njob);
}

Problema* Problema::alloc()
{
	return new JobShop();
}

Problema* Problema::alloc(int** prob)
{
	return new JobShop(prob);
}

Problema* Problema::alloc(Problema& prob)
{
	return new JobShop(prob);
}

Problema* Problema::alloc(Problema& prob, int maq, int pos1, int pos2)
{
	return new JobShop(prob, maq, pos1, pos2);
}


void Problema::leProblema(FILE *f)
{
	if(!fgets (JobShop::name, 128, f))
		exit(1);

	if(!fscanf (f, "%d %d", &JobShop::njob, &JobShop::nmaq))
		exit(1);

	JobShop::maq = (int**)alocaMatriz(2, JobShop::njob, JobShop::nmaq, 0);
	JobShop::time = (int**)alocaMatriz(2, JobShop::njob, JobShop::nmaq, 0);

	for (int i = 0; i < JobShop::njob; i++)
	{
		for (int j = 0; j < JobShop::nmaq; j++)
		{
			if (!fscanf (f, "%d %d", &JobShop::maq[i][j], &JobShop::time[i][j]))
				exit(1);
		}
	}
}

void Problema::leParametros(FILE *f, ParametrosATEAMS *pATEAMS, ParametrosBT *pBT, ParametrosAG *pAG)
{
	char *parametros = (char*)malloc(4097 * sizeof(char));
	size_t size = fread(parametros, sizeof(char), 4096, f);
	float par = -1;

	par = locNumberPar(parametros, size, (char*)"[iterAteams]");
	pATEAMS->iteracoesAteams = par != -1 ? (int)par : 100;

	par = locNumberPar(parametros, size, (char*)"[MaxTempoAteams]");
	pATEAMS->maxTempo = par;

	par = locNumberPar(parametros, size, (char*)"[tamPopulacaoAteams]");
	pATEAMS->tamanhoPopulacao = par != -1 ? (int)par : 50;

	par = locNumberPar(parametros, size, (char*)"[makespanBest]");
	pATEAMS->makespanBest = par;


	par = locNumberPar(parametros, size, (char*)"[probBT]");
	pBT->probBT = par != -1 ? par : (int)50;

	par = locNumberPar(parametros, size, (char*)"[polEscolhaBT]");
	pBT->polEscolha = par != -1 ? par : (int)15;

	par = locNumberPar(parametros, size, (char*)"[funcAspiracaoBT]");
	pBT->funcAsp = par != -1 ? par : 1;

	par = locNumberPar(parametros, size, (char*)"[iterBT]");
	pBT->numeroIteracoes = par != -1 ? (int)par : 1000;

	par = locNumberPar(parametros, size, (char*)"[tentSemMelhoraBT]");
	pBT->tentativasSemMelhora = par != -1 ? (int)par : 1;

	par = locNumberPar(parametros, size, (char*)"[tamListaBT]");
	pBT->tamanhoListaTabu = par != -1 ? (int)par : 2;

	free(parametros);
}

/* Le argumentos adicionais passados por linha de comando */
void Problema::leArgumentos(char **argv, int argc, ParametrosATEAMS *pATEAMS, ParametrosBT *pBT, ParametrosAG *pAG)
{
	int p = -1;

	if((p = locComPar(argv, argc, (char*)"--iterAteams")) != -1)
		pATEAMS->iteracoesAteams = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--MaxTempoAteams")) != -1)
		pATEAMS->maxTempo = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--tamPopulacaoAteams")) != -1)
		pATEAMS->tamanhoPopulacao = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--makespanBest")) != -1)
		pATEAMS->makespanBest = atoi(argv[p]);


	if((p = locComPar(argv, argc, (char*)"--probBT")) != -1)
		pBT->probBT = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--polEscolhaBT")) != -1)
		pBT->polEscolha = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--funcAspiracaoBT")) != -1)
		pBT->funcAsp = atof(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--iterBT")) != -1)
		pBT->numeroIteracoes = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--tentSemMelhoraBT")) != -1)
		pBT->tentativasSemMelhora = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--tamListaBT")) != -1)
		pBT->tamanhoListaTabu = atoi(argv[p]);
}

void Problema::imprimeResultado (struct timeval tv1, struct timeval tv2, FILE *resultados, int bestMakespan)
{
	int s = (((tv2.tv_sec*1000)+(tv2.tv_usec/1000)) - ((tv1.tv_sec*1000)+(tv1.tv_usec/1000)))/1000;

	fprintf(resultados, "%d\t%d\n", bestMakespan, s);
}

void Problema::desalocaMemoria()
{
	desalocaMatriz(2, JobShop::maq, JobShop::njob, 0);
	desalocaMatriz(2, JobShop::time, JobShop::njob, 0);
}

bool Problema::movTabuCMP(mov& t1, mov& t2)
{
	if(t1.maq == t2.maq && (t1.A == t2.A || t1.A == t2.B) && (t1.B == t2.B || t1.B == t2.A))
		return true;
	else
		return false;
}

double Problema::sumFitness(set<Problema*, bool(*)(Problema*, Problema*)> *pop, int n)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	double sum = 0, i = 0;

	for(i = 0, iter = pop->begin(); i < n && iter != pop->end(); i++, iter++)
		sum += (*iter)->getFitness();

	return sum;
}

/* Metodos */

JobShop::JobShop() : Problema::Problema()
{
	int *aux_vet, *aux_maq;
	ptrdiff_t (*p_myrandom)(ptrdiff_t) = myrandom;

	sol.esc = (int**)alocaMatriz(2, nmaq, njob, 0);
	aux_vet = (int*)alocaMatriz(1, njob, 0, 0);
	aux_maq = (int*)alocaMatriz(1, nmaq, 0, 0); /* indica proxima operacao da maquina */

	for (int i = 0; i < nmaq; i++)
		aux_maq[i] = 0;

	for (int i = 0; i < njob; i++)
		aux_vet[i] = i;

	for (int i = 0; i < nmaq; i++)
	{
		random_shuffle(&aux_vet[0], &aux_vet[njob], p_myrandom);
		for (int j = 0; j < njob; j++)
		{
			sol.esc[maq[aux_vet[j]][i]][aux_maq[maq[aux_vet[j]][i]]] = aux_vet[j];
			aux_maq[maq[aux_vet[j]][i]] += 1;
		}
	}

	sol.escalon = NULL;
	sol.makespan = calcMakespan();

	movTabu.job = false;
	movTabu.maq = -1;
}

JobShop::JobShop(int **prob) : Problema::Problema()
{
	sol.esc = (int**)alocaMatriz(2, nmaq, njob, 0);

	for(int i = 0; i < nmaq; i++)
		for(int j = 0; j < njob; j++)
			sol.esc[i][j] = prob[i][j];

	sol.escalon = NULL;
	sol.makespan = calcMakespan();

	movTabu.job = false;
	movTabu.maq = -1;
}

JobShop::JobShop(Problema &prob) : Problema::Problema()
{
	sol.esc = (int**)alocaMatriz(2, nmaq, njob, 0);
	for(int i = 0; i < nmaq; i++)
		for(int j = 0; j < njob; j++)
			sol.esc[i][j] = prob.sol.esc[i][j];

	sol.makespan = prob.sol.makespan;

	if(prob.sol.escalon != NULL)
	{
		sol.escalon = (int***)alocaMatriz(3, nmaq, njob, 3);
		for(int i = 0; i < nmaq; i++)
			for(int j = 0; j < njob; j++)
				for(int k = 0; k < 3; k++)
					sol.escalon[i][j][k] = prob.sol.escalon[i][j][k];
	}
	movTabu = prob.movTabu;
}

JobShop::JobShop(Problema &prob, int maq, int pos1, int pos2) : Problema::Problema()
{
	sol.esc = (int**)alocaMatriz(2, nmaq, njob, 0);
	for(int i = 0; i < nmaq; i++)
		for(int j = 0; j < njob; j++)
			sol.esc[i][j] = prob.sol.esc[i][j];

	int aux = sol.esc[maq][pos1];
	sol.esc[maq][pos1] = sol.esc[maq][pos2];
	sol.esc[maq][pos2] = aux;

	sol.escalon = NULL;
	sol.makespan = calcMakespan();

	movTabu.maq = maq;
	movTabu.A = pos1;
	movTabu.B = pos2;
	movTabu.job = true;
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
int JobShop::calcMakespan()
{
	int ***aux_esc, **tmp, *pos, i, j, k, max, cont;
	int ajob, apos, ainic, afim, sum_time;

	pos = (int*)alocaMatriz(1, nmaq, 0, 0);
	tmp = (int**)alocaMatriz(2, njob, nmaq+1, 0);
	aux_esc = (int***)alocaMatriz(3, nmaq, njob, 3);

	for (i = 0; i < njob; i++)
		for(j = 0; j <= nmaq; j++)
			tmp[i][j] = j == 0 ? 0 : -1;

	for (i = 0; i < nmaq; i++)
		for(j = 0; j < njob; j++)
			aux_esc[i][j][0] = -1;

	for(i = 0; i < nmaq; i++)
		pos[i] = 0;

	cont = i = j = k = 0;
	max = nmaq * njob;

	while((cont < max) && k <= njob)
	{
		if(pos[i] != -1)
		{
			ajob = sol.esc[i][pos[i]];
			apos = findOrdem(nmaq, i, maq[ajob]);
			ainic = tmp[ajob][apos];
		}
		else
			ainic = -1;

		if(ainic != -1)
		{
			cont++;
			k = 0;

			aux_esc[i][pos[i]][0] = ajob;
			if(pos[i] == 0)
			{
				aux_esc[i][pos[i]][1] = ainic;
			}
			else
			{
				afim = aux_esc[i][pos[i] - 1][2];

				aux_esc[i][pos[i]][1] = afim > ainic ? afim : ainic;
			}
			aux_esc[i][pos[i]][2] = aux_esc[i][pos[i]][1] + time[ajob][apos];

			tmp[ajob][apos+1] = aux_esc[i][pos[i]][2];

			pos[i] = pos[i] == njob - 1 ? -1 : pos[i] + 1;
		}
		else
		{
			k++;
			i = (i + 1) % nmaq;
		}
	}

	if(k != njob + 1)
	{
		for(sum_time = 0, i = 0; i < njob; i++)
		{
			if(tmp[i][nmaq] > sum_time)
				sum_time = tmp[i][nmaq];
		}
		sol.escalon = aux_esc;

		desalocaMatriz(2, tmp, njob, 0);
		desalocaMatriz(1, pos, 0, 0);

		return sum_time;
	}
	else
	{
		desalocaMatriz(3, aux_esc, nmaq, njob);
		desalocaMatriz(2, tmp, njob, 0);
		desalocaMatriz(1, pos, 0, 0);

		return -1;
	}
}

void JobShop::imprimir()
{
	if(sol.escalon == NULL)
		calcMakespan();

	printf("\n");

	for(int i = 0; i < nmaq; i++)
	{
		printf("maq %d: ", i+1);
		for(int j = 0; j < njob; j++)
		{
			int k = sol.escalon[i][j][2] - sol.escalon[i][j][1];
			int spc = j == 0 ? sol.escalon[i][j][1] : sol.escalon[i][j][1] - sol.escalon[i][j-1][2];
			while(spc--)
				printf(" ");

			while(k--)
				printf("%d", sol.escalon[i][j][0]+1);
		}
		printf("\n");
	}
	return;
}

multiset<Problema*, bool(*)(Problema*, Problema*)>* JobShop::buscaLocal()
{
	multiset<Problema*, bool(*)(Problema*, Problema*)>* local;
	bool(*fn_pt)(Problema*, Problema*) = fncomp;
	local = new multiset<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	Problema *job = NULL;
	for(int i = 0; i < nmaq; i++)
	{
		for(int j = 0; j < njob-1; j++)
		{
			for(int k = j+1; k < njob; k++)
			{
				job = new JobShop(*this, i, j, k);
				if(job->sol.makespan != -1)
					local->insert(job);
				else
					delete job;
			}
		}
	}

	return local;
}

double JobShop::getFitness()
{
	return (double)1000/sol.makespan;
}

int JobShop::getMakespan()
{
	return sol.makespan;
}


/* Auxiliares */

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

float locNumberPar(char *in, int num, char *key)
{
	char *str = locPosPar(in, num, key);
	float ret = -1;

	if(str != NULL)
		sscanf(str, "%f", &ret);

	return ret;
}

char* locPosPar(char *in, int num, char *key)
{
	char *str = strstr(in, key);

	if(str != NULL)
		return strstr(str, "=") + 1;
	else
		return NULL;
}

int findOrdem(int M, int maq, int* job)
{
	for(int i = 0; i < M; i++)
		if(job[i] == maq)
			return i;
	return -1;
}

void* alocaMatriz(int dim, int a, int b, int c)
										{
	if(dim == 1)
	{
		int *M = (int*)malloc(a * sizeof(int));

		return (void*)M;
	}
	else if(dim == 2)
	{
		int **M = (int**)malloc(a * sizeof (int*));
		for (int i = 0; i < a; i++)
			M[i] = (int*)malloc(b * sizeof (int));

		return (void*)M;
	}
	else if(dim == 3)
	{
		int ***M = (int***)malloc(a * sizeof (int**));
		for (int i = 0; i < a; i++)
		{
			M[i] = (int**)malloc(b * sizeof(int*));
			for(int j = 0; j < b; j++)
				M[i][j] = (int*)malloc(c * sizeof (int));
		}

		return (void*)M;
	}
	else
		return NULL;
										}

void desalocaMatriz(int dim, void *MMM, int a, int b)
{
	if(dim == 1)
	{
		int *M = (int*)MMM;

		free(M);
	}
	else if(dim == 2)
	{
		int **M = (int**)MMM;

		for(int i = 0; i < a; i++)
			free(M[i]);
		free(M);
	}
	else if(dim == 3)
	{
		int ***M = (int***)MMM;

		for(int i = 0; i < a; i++)
		{
			for(int j = 0; j < b; j++)
				free(M[i][j]);
			free(M[i]);
		}
		free(M);
	}
	return;
}

// comparator function:
bool fncomp(Problema *prob1, Problema *prob2)
{
	if(prob1->sol.makespan == prob2->sol.makespan)
	{
		for(int i = 0; i < JobShop::nmaq; i++)
			for(int j = 0; j < JobShop::njob; j++)
				if(prob1->sol.esc[i][j] != prob2->sol.esc[i][j])
					return prob1->sol.esc[i][j] < prob2->sol.esc[i][j];
		return false;
	}
	else
		return prob1->sol.makespan < prob2->sol.makespan;
}

// random generator function:
ptrdiff_t myrandom (ptrdiff_t i)
{
	return rand() % i;
}
