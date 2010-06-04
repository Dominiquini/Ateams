#include "JobShop.h"

using namespace std;

/* Static Members */

int Problema::best = 0;
int Problema::worst = 0;
int Problema::numInst = 0;
long int Problema::totalNumInst = 0;

char JobShop::name[128];
short int **JobShop::maq = NULL, **JobShop::time = NULL;
int JobShop::njob = 0, JobShop::nmaq = 0;


Problema* Problema::alloc()
{
	return new JobShop();
}

Problema* Problema::alloc(const Problema& prob)
{
	return new JobShop(prob);
}


void Problema::leProblema(FILE *f)
{
	if(!fgets (JobShop::name, 128, f))
		exit(1);

	if(!fscanf (f, "%d %d", &JobShop::njob, &JobShop::nmaq))
		exit(1);

	JobShop::maq = (short int**)alocaMatriz(2, JobShop::njob, JobShop::nmaq, 1);
	JobShop::time = (short int**)alocaMatriz(2, JobShop::njob, JobShop::nmaq, 1);

	for (int i = 0; i < JobShop::njob; i++)
	{
		for (int j = 0; j < JobShop::nmaq; j++)
		{
			if (!fscanf (f, "%hd %hd", &JobShop::maq[i][j], &JobShop::time[i][j]))
				exit(1);
		}
	}
}

void Problema::leParametros(FILE *f, ParametrosATEAMS *pATEAMS, vector<ParametrosHeuristicas> *pHEURISTICAS)
{
	char *parametros = (char*)malloc(4097 * sizeof(char)), *algs[16], *pos;
	size_t size = fread(parametros, sizeof(char), 4096, f);
	int numAlgs = 0;
	float par = -1;

	par = locNumberPar(parametros, size, (char*)"[iterAteams]");
	pATEAMS->iteracoesAteams = (int)par;

	par = locNumberPar(parametros, size, (char*)"[numThreads]");
	pATEAMS->numThreads = (int)par;

	par = locNumberPar(parametros, size, (char*)"[maxTempoAteams]");
	pATEAMS->maxTempo = (int)par;

	par = locNumberPar(parametros, size, (char*)"[tamPopulacaoAteams]");
	pATEAMS->tamanhoPopulacao = (int)par;

	par = locNumberPar(parametros, size, (char*)"[makespanBest]");
	pATEAMS->makespanBest = (int)par;

	pos = strstr(parametros, (char*)"##(");
	while (pos != NULL)
	{
		*pos = '\0';
		algs[numAlgs++] = pos+1;
		parametros = pos+7;
		pos = strstr(parametros, (char*)"##(");
	}

	ParametrosHeuristicas pTEMP;
	for(int i = 0; i < numAlgs; i++)
	{
		parametros = algs[i];

		pos = strstr(parametros, (char*)"(name) = ");
		pos = pos + strlen((char*)"(name) = ");
		sscanf(pos, "%s", pTEMP.algName);

		if(strstr(parametros, (char*)"#(SA)#"))
		{
			pTEMP.alg = SA;

			par = locNumberPar(parametros, size, (char*)"[probSA]");
			pTEMP.probSA = (float)par;

			par = locNumberPar(parametros, size, (char*)"[polEscolhaSA]");
			pTEMP.polEscolhaSA = (int)par;

			par = locNumberPar(parametros, size, (char*)"[maxIterSA]");
			pTEMP.maxIterSA = (int)par;

			par = locNumberPar(parametros, size, (char*)"[initTempSA]");
			pTEMP.initTempSA = (float)par;

			par = locNumberPar(parametros, size, (char*)"[finalTempSA]");
			pTEMP.finalTempSA = (float)par;

			par = locNumberPar(parametros, size, (char*)"[restauraSolSA]");
			pTEMP.restauraSolSA = (int)par;

			par = locNumberPar(parametros, size, (char*)"[alphaSA]");
			pTEMP.alphaSA = (float)par;
		}
		else if(strstr(parametros, (char*)"#(AG)#"))
		{
			pTEMP.alg = AG;

			par = locNumberPar(parametros, size, (char*)"[probAG]");
			pTEMP.probAG = (int)par;

			par = locNumberPar(parametros, size, (char*)"[polEscolhaAG]");
			pTEMP.polEscolhaAG = (int)par;

			par = locNumberPar(parametros, size, (char*)"[iterAG]");
			pTEMP.iterAG = (int)par;

			par = locNumberPar(parametros, size, (char*)"[tamPopAG]");
			pTEMP.tamPopAG = (int)par;

			par = locNumberPar(parametros, size, (char*)"[tamParticaoAG]");
			pTEMP.tamParticaoAG = (int)par;

			par = locNumberPar(parametros, size, (char*)"[probCrossOverAG]");
			pTEMP.probCrossOverAG = (float)par;

			par = locNumberPar(parametros, size, (char*)"[probMutacaoAG]");
			pTEMP.probMutacaoAG = (float)par;
		}
		else if(strstr(parametros, (char*)"#(BT)#"))
		{
			pTEMP.alg = BT;

			par = locNumberPar(parametros, size, (char*)"[probBT]");
			pTEMP.probBT = (int)par;

			par = locNumberPar(parametros, size, (char*)"[polEscolhaBT]");
			pTEMP.polEscolhaBT = (int)par;

			par = locNumberPar(parametros, size, (char*)"[iterBT]");
			pTEMP.iterBT = (int)par;

			par = locNumberPar(parametros, size, (char*)"[tentSemMelhoraBT]");
			pTEMP.tentSemMelhoraBT = (int)par;

			par = locNumberPar(parametros, size, (char*)"[tamListaBT]");
			pTEMP.tamListaBT = (int)par;

			par = locNumberPar(parametros, size, (char*)"[polExplorBT]");
			pTEMP.polExplorBT = (float)par;

			par = locNumberPar(parametros, size, (char*)"[funcAspiracaoBT]");
			pTEMP.funcAspiracaoBT = (float)par;
		}
		else
		{
			printf("\n\nERRO NO ARQUIVO DE PARÂMETROS!!!\n\n");
			exit(0);
		}
		pHEURISTICAS->push_back(pTEMP);
	}
}

/* Le argumentos adicionais passados por linha de comando */
void Problema::leArgumentos(char **argv, int argc, ParametrosATEAMS *pATEAMS)
{
	int p = -1;

	if((p = locComPar(argv, argc, (char*)"--iterAteams")) != -1)
		pATEAMS->iteracoesAteams = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--numThreads")) != -1)
		pATEAMS->numThreads = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--maxTempoAteams")) != -1)
		pATEAMS->maxTempo = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--tamPopulacaoAteams")) != -1)
		pATEAMS->tamanhoPopulacao = atoi(argv[p]);

	if((p = locComPar(argv, argc, (char*)"--makespanBest")) != -1)
		pATEAMS->makespanBest = atoi(argv[p]);
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

double Problema::sumFitnessMaximize(set<Problema*, bool(*)(Problema*, Problema*)> *pop, int n)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	double sum = 0, i = 0;

	for(i = 0, iter = pop->begin(); i < n && iter != pop->end(); i++, iter++)
		sum += (*iter)->getFitnessMaximize();

	return sum;
}

double Problema::sumFitnessMaximize(vector<Problema*> *pop, int n)
{
	vector<Problema*>::iterator iter;
	double sum = 0, i = 0;

	for(i = 0, iter = pop->begin(); i < n && iter != pop->end(); i++, iter++)
		sum += (*iter)->getFitnessMaximize();

	return sum;
}

double Problema::sumFitnessMinimize(set<Problema*, bool(*)(Problema*, Problema*)> *pop, int n)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	double sum = 0, i = 0;

	for(i = 0, iter = pop->begin(); i < n && iter != pop->end(); i++, iter++)
		sum += (*iter)->getFitnessMinimize();

	return sum;
}

double Problema::sumFitnessMinimize(vector<Problema*> *pop, int n)
{
	vector<Problema*>::iterator iter;
	double sum = 0, i = 0;

	for(i = 0, iter = pop->begin(); i < n && iter != pop->end(); i++, iter++)
		sum += (*iter)->getFitnessMinimize();

	return sum;
}

/* Metodos */

JobShop::JobShop() : Problema::Problema()
{
	short int *aux_vet, *aux_maq;

	sol.esc = (short int**)alocaMatriz(2, nmaq, njob, 1);
	aux_vet = (short int*)alocaMatriz(1, njob, 1, 1);
	aux_maq = (short int*)alocaMatriz(1, nmaq, 1, 1); 	// Indica proxima operacao da maquina

	for (int i = 0; i < nmaq; i++)
		aux_maq[i] = 0;

	for (int i = 0; i < njob; i++)
		aux_vet[i] = i;

	for (int i = 0; i < nmaq; i++)
	{
		random_shuffle(&aux_vet[0], &aux_vet[njob]);
		for (int j = 0; j < njob; j++)
		{
			sol.esc[maq[aux_vet[j]][i]][aux_maq[maq[aux_vet[j]][i]]] = aux_vet[j];
			aux_maq[maq[aux_vet[j]][i]] += 1;
		}
	}

	sol.makespan = calcMakespan();

#ifndef ESCALONAMENTO
	if(sol.makespan != -1)
	{
		desalocaMatriz(3, sol.escalon, nmaq, njob);
		sol.escalon = NULL;
	}
#endif

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}


JobShop::JobShop(short int **prob) : Problema::Problema()
{
	sol.esc = prob;

	sol.escalon = NULL;
	sol.makespan = calcMakespan();

#ifndef ESCALONAMENTO
	if(sol.makespan != -1)
	{
		desalocaMatriz(3, sol.escalon, nmaq, njob);
		sol.escalon = NULL;
	}
#endif

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

JobShop::JobShop(const Problema &prob) : Problema::Problema()
{
	sol.esc = (short int**)alocaMatriz(2, nmaq, njob, 1);
	for(int i = 0; i < nmaq; i++)
		for(int j = 0; j < njob; j++)
			sol.esc[i][j] = prob.sol.esc[i][j];

	sol.makespan = prob.sol.makespan;

#ifdef ESCALONAMENTO
	if(prob.sol.escalon != NULL)
	{
		sol.escalon = (short int***)alocaMatriz(3, nmaq, njob, 3);
		for(int i = 0; i < nmaq; i++)
			for(int j = 0; j < njob; j++)
				for(int k = 0; k < 3; k++)
					sol.escalon[i][j][k] = prob.sol.escalon[i][j][k];
	}
#else
	sol.escalon = NULL;
#endif

	exec = prob.exec;
}

JobShop::JobShop(const Problema &prob, int maq, int pos1, int pos2) : Problema::Problema()
{
	sol.esc = (short int**)alocaMatriz(2, nmaq, njob, 1);
	for(int i = 0; i < nmaq; i++)
		for(int j = 0; j < njob; j++)
			sol.esc[i][j] = prob.sol.esc[i][j];

	int aux = sol.esc[maq][pos1];
	sol.esc[maq][pos1] = sol.esc[maq][pos2];
	sol.esc[maq][pos2] = aux;

	sol.escalon = NULL;
	sol.makespan = calcMakespan();

#ifndef ESCALONAMENTO
	if(sol.makespan != -1)
	{
		desalocaMatriz(3, sol.escalon, nmaq, njob);
		sol.escalon = NULL;
	}
#endif

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

JobShop::~JobShop()
{
	if(sol.esc != NULL)
		desalocaMatriz(2, sol.esc, JobShop::nmaq, 0);

	if(sol.escalon != NULL)
		desalocaMatriz(3, sol.escalon, JobShop::nmaq, JobShop::njob);
}

bool JobShop::operator == (Problema& p)
		{
	return fnequal1(this, &p);
		}

bool JobShop::operator != (Problema& p)
		{
	return fnequal2(this, &p);
		}

bool JobShop::operator <= (Problema& p)
		{
	return fncomp1(this, &p);
		}

bool JobShop::operator >= (Problema& p)
		{
	return fncomp1(&p, this);
		}

bool JobShop::operator < (Problema& p)
{
	return fncomp2(this, &p);
}

bool JobShop::operator > (Problema& p)
{
	return fncomp1(&p, this);
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
inline int JobShop::calcMakespan()
{
	register int max, cont = 0;
	short int ***aux_esc, **tmp, *pos;
	int ajob, apos, ainic, afim, sum_time;

	pos = (short int*)alocaMatriz(1, nmaq, 1, 1);
	tmp = (short int**)alocaMatriz(2, njob, nmaq+1, 1);
	aux_esc = (short int***)alocaMatriz(3, nmaq, njob, 3);

	register int i, j, k;

	for (i = 0; i < njob; i++)
		for(j = 0; j <= nmaq; j++)
			tmp[i][j] = j == 0 ? 0 : -1;

	for (i = 0; i < nmaq; i++)
		for(j = 0; j < njob; j++)
			aux_esc[i][j][0] = -1;

	for(i = 0; i < nmaq; i++)
		pos[i] = 0;

	max = nmaq * njob;

	i = 0, k = 0;
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

inline void JobShop::imprimir(bool esc)
{
	if(esc == true)
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
	}
	else
	{
		for(int i = 0; i < nmaq; i++)
		{
			for(int j = 0; j < njob; j++)
			{
				printf("%.2d ", sol.esc[i][j]);
			}
			printf("\n");
		}
	}
	return;
}

inline Problema* JobShop::vizinho()
{
	int maq = xRand(rand(), 0, nmaq), p1 = xRand(rand(), 0, njob), p2 = xRand(rand(), 0, njob);

	while(p2 == p1)
		p2 = xRand(rand(), 0, njob);

	return new JobShop(*this, maq, p1, p2);
}

inline vector<pair<Problema*, movTabu*>* >* JobShop::buscaLocal()
{
	Problema *job = NULL;
	register int maq, p1, p2;
	int numMaqs = nmaq, numJobs = njob;
	pair<Problema*, movTabu*>* temp;
	vector<pair<Problema*, movTabu*>* >* local = new vector<pair<Problema*, movTabu*>* >();

#pragma omp parallel for shared(local, numMaqs, numJobs) private(maq, p1, p2, job, temp)
	for(maq = 0; maq < numMaqs; maq++)
	{
		for(p1 = 0; p1 < numJobs-1; p1++)
		{
			for(p2 = p1+1; p2 < numJobs; p2++)
			{
				job = new JobShop(*this, maq, p1, p2);
				if(job->sol.makespan != -1)
				{
					temp = new pair<Problema*, movTabu*>();
					temp->first = job;
					temp->second = new movTabu(maq, p1, p2);

#pragma omp critical
					{
						local->push_back(temp);
					}
				}
				else
				{
					delete job;
				}
			}
		}
	}
	random_shuffle(local->begin(), local->end());
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

inline vector<pair<Problema*, movTabu*>* >* JobShop::buscaLocal(float parcela)
{
	Problema *job = NULL;
	int maq, p1, p2;
	int numMaqs = nmaq, numJobs = njob;
	pair<Problema*, movTabu*>* temp;
	vector<pair<Problema*, movTabu*>* >* local = new vector<pair<Problema*, movTabu*>* >();
	int total, def, i;

	for(total = 0, i = njob; i > 0; i--)
		total += i;
	total *= nmaq;

	def = (int)((float)total*parcela);

#pragma omp parallel for shared(local, numMaqs, numJobs, def) private(maq, p1, p2, job, temp, i)
	for(i = 0; i < def; i++)
	{
		maq = xRand(rand(), 0, numMaqs), p1 = xRand(rand(), 0, numJobs), p2 = xRand(rand(), 0, numJobs);

		while(p2 == p1)
			p2 = xRand(rand(), 0, numJobs);

		job = new JobShop(*this, maq, p1, p2);
		if(job->sol.makespan != -1)
		{
			temp = new pair<Problema*, movTabu*>();
			temp->first = job;
			temp->second = new movTabu(maq, p1, p2);

#pragma omp critical
			{
				local->push_back(temp);
			}
		}
		else
		{
			delete job;
		}
	}
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

inline pair<Problema*, Problema*>* JobShop::crossOver(Problema* pai, int tamParticao)
{
	short int **f1 = (short int**)alocaMatriz(2, nmaq, njob, 1), **f2 = (short int**)alocaMatriz(2, nmaq, njob, 1);
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = tamParticao == -1 ? (JobShop::njob)/2 : tamParticao;
	int inicioPart = 0, fimPart = 0;

	for(register int i = 0; i < nmaq; i++)
	{
		inicioPart = xRand(rand(), 0, njob);
		fimPart = inicioPart+particao <= njob ? inicioPart+particao : njob;

		swap_vect(this->sol.esc[i], pai->sol.esc[i], f1[i], inicioPart, fimPart-inicioPart);
		swap_vect(pai->sol.esc[i], this->sol.esc[i], f2[i], inicioPart, fimPart-inicioPart);
	}

	filhos->first = new JobShop(f1);
	filhos->second = new JobShop(f2);

	return filhos;
}

inline pair<Problema*, Problema*>* JobShop::crossOver(Problema* pai)
{
	short int **f1 = (short int**)alocaMatriz(2, nmaq, njob, 1), **f2 = (short int**)alocaMatriz(2, nmaq, njob, 1);
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = 0;

	for(register int i = 0; i < nmaq; i++)
	{
		particao = xRand(rand(), 1, njob);

		swap_vect(this->sol.esc[i], pai->sol.esc[i], f1[i], 0, particao);
		swap_vect(pai->sol.esc[i], this->sol.esc[i], f2[i], 0, particao);
	}

	filhos->first = new JobShop(f1);
	filhos->second = new JobShop(f2);

	return filhos;
}

inline void JobShop::mutacao()
{
	int maq = xRand(rand(), 0, nmaq);
	int pos1 = xRand(rand(), 0, njob), pos2 = xRand(rand(), 0, njob);

	while(pos2 == pos1)
		pos2 = xRand(rand(), 0, njob);

	short int aux = sol.esc[maq][pos1];
	sol.esc[maq][pos1] = sol.esc[maq][pos2];
	sol.esc[maq][pos2] = aux;

	if(sol.makespan != -1 && sol.escalon != NULL)
	{
		desalocaMatriz(3, sol.escalon, nmaq, njob);
		sol.escalon = NULL;
	}

	sol.makespan = calcMakespan();

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

inline double JobShop::getFitnessMaximize()
{
	return (double)INV_FITNESS/sol.makespan;
}

inline double JobShop::getFitnessMinimize()
{
	return (double)sol.makespan;
}

inline int** JobShop::getEscalonameto()
{
	int** copy = (int**)alocaMatriz(2, nmaq, njob, 1);
	for(int i = 0; i < nmaq; i++)
		for(int j = 0; j < njob; j++)
			copy[i][j] = this->sol.esc[i][j];

	return copy;
}

/* Auxiliares */

inline void swap_vect(short int* p1, short int* p2, short int* f, int pos, int tam)
{
	for(int i = pos; i < pos+tam; i++)
		f[i] = p1[i];

	for(register int i = 0, j = 0; i < JobShop::njob && j < JobShop::njob; i++)
	{
		if(j == pos)
			j = pos+tam;

		if(find(&p1[pos], &p1[pos+tam], p2[i]) == &p1[pos+tam])
			f[j++] = p2[i];
	}

	return;
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

int findOrdem(int M, int maq, short int* job)
{
	for(int i = 0; i < M; i++)
		if(job[i] == maq)
			return i;
	return -1;
}

inline void* alocaMatriz(int dim, int a, int b, int c)
						{
	if(dim == 1)
	{
		short int *M = (short int*)malloc(a * sizeof(short int));

		return (void*)M;
	}
	else if(dim == 2)
	{
		short int **M = (short int**)malloc(a * sizeof (short int*));
		for (int i = 0; i < a; i++)
			M[i] = (short int*)malloc(b * sizeof (short int));

		return (void*)M;
	}
	else if(dim == 3)
	{
		short int ***M = (short int***)malloc(a * sizeof (short int**));
		for (int i = 0; i < a; i++)
		{
			M[i] = (short int**)malloc(b * sizeof(short int*));
			for(int j = 0; j < b; j++)
				M[i][j] = (short int*)malloc(c * sizeof (short int));
		}

		return (void*)M;
	}
	else
		return NULL;
						}

inline void desalocaMatriz(int dim, void *MMM, int a, int b)
{
	if(dim == 1)
	{
		short int *M = (short int*)MMM;

		free(M);
	}
	else if(dim == 2)
	{
		short int **M = (short int**)MMM;

		for(int i = 0; i < a; i++)
			free(M[i]);
		free(M);
	}
	else if(dim == 3)
	{
		short int ***M = (short int***)MMM;

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
bool fnequal1(Problema* p1, Problema* p2)
{
	if(p1->sol.makespan == p2->sol.makespan)
	{
		for(int i = 0; i < JobShop::nmaq; i++)
			for(int j = 0; j < JobShop::njob; j++)
				if(p1->sol.esc[i][j] != p2->sol.esc[i][j])
					return false;
		return true;
	}
	else
		return false;
}

// comparator function:
bool fnequal2(Problema* p1, Problema* p2)
{
	return p1->sol.makespan == p2->sol.makespan;
}

// comparator function:
bool fncomp1(Problema *prob1, Problema *prob2)
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

// comparator function:
bool fncomp2(Problema *prob1, Problema *prob2)
{
	return prob1->sol.makespan < prob2->sol.makespan;
}


inline bool ptcomp(pair<Problema*, movTabu*>* p1, pair<Problema*, movTabu*>* p2)
{
	return (p1->first->getFitnessMinimize() > p2->first->getFitnessMinimize());
}

inline bool find(vector<Problema*> *vect, Problema *p)
{
	vector<Problema*>::iterator iter;

	for(iter = vect->begin(); iter != vect->end(); iter++)
		if(fnequal1((*iter), p))
			return true;

	return false;
}
