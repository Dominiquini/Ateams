#include "JobShop.h"

using namespace std;

/* Static Members */

int Problema::numInst = 0;
char Problema::name[128];
int **Problema::maq = NULL, **Problema::time = NULL;
int Problema::njob = 0, Problema::nmaq = 0;


Problema::Problema()
{
	numInst++;
}

Problema::~Problema()
{
	numInst--;

	if(esc != NULL)
		desalocaMatriz(2, esc, nmaq, 0);

	if(escalon != NULL)
		desalocaMatriz(3, escalon, nmaq, njob);
}


void Problema::leProblema(FILE *file)
{
	if(!fgets (name, 128, file))
		exit(1);
	if(!fscanf (file, "%d %d", &njob, &nmaq))
		exit(1);
	maq = (int**)alocaMatriz(2, njob, nmaq, 0);
	time = (int**)alocaMatriz(2, njob, nmaq, 0);
	for (int i = 0; i < njob; i++) {
		for (int j = 0; j < nmaq; j++) {
			if (!fscanf (file, "%d %d", &maq[i][j], &time[i][j]))
				exit(1);
		}
	}
}


/* Metodos */

JobShop::JobShop() : Problema::Problema()
{
	int *aux_vet, *aux_maq;
	ptrdiff_t (*p_myrandom)(ptrdiff_t) = myrandom;

	esc = (int**)alocaMatriz(2, nmaq, njob, 0);
	aux_vet = (int*)alocaMatriz(1, njob, 0, 0);
	aux_maq = (int*)alocaMatriz(1, nmaq, 0, 0); /* indica proxima operacao da maquina */

	for (int i = 0; i < nmaq; i++)
		aux_maq[i] = 0;
	for (int i = 0; i < njob; i++)
		aux_vet[i] = i;
	for (int i = 0; i < nmaq; i++) {
		random_shuffle(&aux_vet[0], &aux_vet[njob], p_myrandom);
		for (int j = 0; j < njob; j++) {
			esc[maq[aux_vet[j]][i]][aux_maq[maq[aux_vet[j]][i]]] = aux_vet[j];
			aux_maq[maq[aux_vet[j]][i]] += 1;
		}
	}
	escalon = NULL;
	makespan = calcMakespan();
}

JobShop::JobShop(int **prob) : Problema::Problema()
{
	esc = (int**)alocaMatriz(2, nmaq, njob, 0);

	for(int i = 0; i < nmaq; i++)
		for(int j = 0; j < njob; j++)
			esc[i][j] = prob[i][j];

	escalon = NULL;
	makespan = calcMakespan();
}

JobShop::JobShop(JobShop &prob) : Problema::Problema()
{
	esc = (int**)alocaMatriz(2, nmaq, njob, 0);
	for(int i = 0; i < nmaq; i++)
		for(int j = 0; j < njob; j++)
			esc[i][j] = prob.esc[i][j];

	makespan = prob.makespan;

	if(prob.escalon != NULL)
	{
		escalon = (int***)alocaMatriz(3, nmaq, njob, 3);
		for(int i = 0; i < nmaq; i++)
			for(int j = 0; j < njob; j++)
				for(int k = 0; k < 3; k++)
					escalon[i][j][k] = prob.escalon[i][j][k];
	}
}

JobShop::JobShop(JobShop &prob, int maq, int pos1, int pos2) : Problema::Problema()
{
	esc = (int**)alocaMatriz(2, nmaq, njob, 0);
	for(int i = 0; i < nmaq; i++)
		for(int j = 0; j < njob; j++)
			esc[i][j] = prob.esc[i][j];

	int aux = esc[maq][pos1];
	esc[maq][pos1] = esc[maq][pos2];
	esc[maq][pos2] = aux;

	escalon = NULL;
	makespan = calcMakespan();
}

/*
 * Devolve o makespan  e o escalonamento quando a solucao for factivel,
 * ou -1 quando for invalido.
 */
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
			ajob = esc[i][pos[i]];
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
		escalon = aux_esc;

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
	if(escalon == NULL)
		calcMakespan();

	printf("\n");

	for(int i = 0; i < nmaq; i++)
	{
		printf("maq %d: ", i+1);
		for(int j = 0; j < njob; j++)
		{
			int k = escalon[i][j][2] - escalon[i][j][1];
			int spc = j == 0 ? escalon[i][j][1] : escalon[i][j][1] - escalon[i][j-1][2];
			while(spc--)
				printf(" ");

			while(k--)
				printf("%d", escalon[i][j][0]+1);
		}
		printf("\n");
	}
	return;
}

/* Auxiliares */

void imprimir(int N, int M, int ***esc)
{
	printf("\n");

	for(int i = 0; i < M; i++)
	{
		printf("maq %d: ", i+1);
		for(int j = 0; j < N; j++)
		{
			int k = esc[i][j][2] - esc[i][j][1];
			int spc = j == 0 ? esc[i][j][1] : esc[i][j][1] - esc[i][j-1][2];
			while(spc--)
				printf(" ");

			while(k--)
				printf("%d", esc[i][j][0]+1);
		}
		printf("\n");
	}
	return;
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
	return prob1->makespan < prob2->makespan;
}

// random generator function:
ptrdiff_t myrandom (ptrdiff_t i)
{
	return rand() % i;
}
