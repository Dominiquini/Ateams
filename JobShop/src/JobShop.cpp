#include "JobShop.h"

using namespace std;

/* Static Members */

ProblemType Problema::TIPO = MINIMIZACAO;

double Problema::best = 0;
double Problema::worst = 0;
int Problema::numInst = 0;
long int Problema::totalNumInst = 0;

char JobShop::name[128];
int **JobShop::maq = NULL, **JobShop::time = NULL;
int JobShop::njob = 0, JobShop::nmaq = 0;

int JobShop::num_vizinhos = 0;

Problema* Problema::randSoluction()
{
	return new JobShop();
}

Problema* Problema::copySoluction(const Problema& prob)
{
	return new JobShop(prob);
}


void Problema::leProblema(FILE *f)
{
	if(!fgets (JobShop::name, 128, f))
		exit(1);

	if(!fscanf (f, "%d %d", &JobShop::njob, &JobShop::nmaq))
		exit(1);

	Problema::alocaMemoria();

	for(int i = 0; i < JobShop::njob; i++)
	{
		for(int j = 0; j < JobShop::nmaq; j++)
		{
			if (!fscanf (f, "%d %d", &JobShop::maq[i][j], &JobShop::time[i][j]))
				exit(1);
		}
	}

	for(int i = 1; i < JobShop::njob; i++)
		JobShop::num_vizinhos += i;
	JobShop::num_vizinhos *= JobShop::nmaq;

	return;
}

list<Problema*>* Problema::lePopulacao(char *log)
{
	FILE *f = fopen(log, "r");

	if(f != NULL)
	{
		list<Problema*>* popInicial = new list<Problema*>();
		int njob, nmaq, nprob, makespan;
		short int **prob;
		Problema* p;

		if(!fscanf (f, "%d %d %d", &nprob, &nmaq, &njob))
		{
			printf("Arquivo de log incorreto!\n\n");
			exit(1);
		}

		if(nmaq != JobShop::nmaq || njob != JobShop::njob)
		{
			printf("Arquivo de log incorreto!\n\n");
			exit(1);
		}

		for(int i = 0; i < nprob; i++)
		{
			prob = (short int**)alocaMatriz(2, nmaq, njob, 1);

			if(!fscanf (f, "%d", &makespan))
			{
				printf("Arquivo de log incorreto!\n\n");
				exit(1);
			}

			for(int i = 0; i < nmaq; i++)
			{
				for(int j = 0; j < njob; j++)
				{
					if(!fscanf (f, "%hd", &prob[i][j]))
					{
						printf("Arquivo de log incorreto!\n\n");
						exit(1);
					}
				}
			}

			p = new JobShop(prob);

			if(makespan != p->getFitness())
			{
				printf("Arquivo de log incorreto!\n\n");
				exit(1);
			}

			popInicial->push_back(p);
		}
		fclose(f);

		return popInicial;
	}
	else
	{
		return NULL;
	}
}

void Problema::escrevePopulacao(char *log, list<Problema*>* popInicial)
{
	FILE *f = fopen(log, "w");

	int sizePop = (int)popInicial->size();
	list<Problema*>::iterator iter;
	short int **prob;

	fprintf(f, "%d %d %d\n\n", sizePop, JobShop::nmaq, JobShop::njob);

	for(iter = popInicial->begin(); iter != popInicial->end(); iter++)
	{
		prob = dynamic_cast<JobShop *>(*iter)->getSoluction().esc;

		fprintf(f, "%d\n", (int)dynamic_cast<JobShop *>(*iter)->getSoluction().fitness);

		for(int j = 0; j < JobShop::nmaq; j++)
		{
			for(int m = 0; m < JobShop::njob; m++)
			{
				fprintf(f, "%.2d ", prob[j][m]);
			}
			fprintf(f, "\n");
		}
		fprintf(f, "\n");
	}
	fclose(f);
}

void Problema::escreveResultado(char *dados, char *parametros, execInfo *info, char *resultado)
{
	FILE *f;

	if((f = fopen(resultado, "r+")) != NULL)
	{
		fseek(f, 0, SEEK_END);
	}
	else
	{
		f = fopen(resultado, "w");

		fprintf(f, "%*s%*s", -16, "bestFitness", -16, "worstFitness");
		fprintf(f, "%*s%*s%*s", -16, "numExecs", -16, "diffTime", -24, "expSol");
		fprintf(f, "%*s%s\n", -24, "dados", "parametros");
	}

	fprintf(f, "%*d%*d", -16, (int)info->bestFitness, -16, (int)info->worstFitness);
	fprintf(f, "%*d%*d%*d", -16, info->numExecs, -16, (int)info->diffTime, -24, (int)info->expSol);
	fprintf(f, "%*s%s\n", -24, dados, parametros);

	fclose(f);
}

void Problema::alocaMemoria()
{
	JobShop::maq = (int**)malloc(JobShop::njob * sizeof(int*));
	for(int i = 0; i < JobShop::njob; i++)
		JobShop::maq[i] = (int*)malloc(JobShop::nmaq * sizeof(int));

	JobShop::time = (int**)malloc(JobShop::njob * sizeof(int*));
	for(int i = 0; i < JobShop::njob; i++)
		JobShop::time[i] = (int*)malloc(JobShop::nmaq * sizeof(int));
}

void Problema::desalocaMemoria()
{
	for(int i = 0; i < JobShop::njob; i++)
		free(JobShop::maq[i]);
	free(JobShop::maq);

	for(int i = 0; i < JobShop::njob; i++)
		free(JobShop::time[i]);
	free(JobShop::time);
}

double Problema::compare(double oldP, double newP)
{
	return oldP - newP;
}

double Problema::compare(Problema& oldP, Problema& newP)
{
	return oldP.getFitness() - newP.getFitness();
}

/* Metodos */

JobShop::JobShop() : Problema::Problema()
{
	sol.esc = (short int**)alocaMatriz(2, nmaq, njob, 1);

	if(rand() % 2 == 0)
	{
		short int *aux_vet = (short int*)alocaMatriz(1, njob, 1, 1);
		short int *aux_maq = (short int*)alocaMatriz(1, nmaq, 1, 1);

		for(int i = 0; i < nmaq; i++)
			aux_maq[i] = 0;

		for(int i = 0; i < njob; i++)
			aux_vet[i] = i;

		for(int i = 0; i < nmaq; i++)
		{
			random_shuffle(&aux_vet[0], &aux_vet[njob]);
			for(int j = 0; j < njob; j++)
			{
				sol.esc[maq[aux_vet[j]][i]][aux_maq[maq[aux_vet[j]][i]]] = aux_vet[j];
				aux_maq[maq[aux_vet[j]][i]] += 1;
			}
		}

		desalocaMatriz(1, aux_vet, 1, 1);
		desalocaMatriz(1, aux_maq, 1, 1);
	}
	else
	{
		for(int i = 0; i < nmaq; i++)
		{
			for(int j = 0; j < njob; j++)
			{
				sol.esc[i][j] = j;
			}
			random_shuffle(&sol.esc[i][0], &sol.esc[i][njob]);
		}
	}

	sol.escalon = NULL;
	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}


JobShop::JobShop(short int **prob) : Problema::Problema()
{
	sol.esc = prob;

	sol.escalon = NULL;
	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

JobShop::JobShop(const Problema &prob) : Problema::Problema()
{
	JobShop *other = dynamic_cast<JobShop *>(const_cast<Problema *>(&prob));

	this->sol.esc = (short int**)alocaMatriz(2, nmaq, njob, 1);
	for(int i = 0; i < nmaq; i++)
		for(int j = 0; j < njob; j++)
			this->sol.esc[i][j] = other->sol.esc[i][j];

	this->sol.escalon = NULL;
	this->sol.fitness = other->sol.fitness;

	if(other->sol.escalon != NULL)
	{
		this->sol.escalon = (short int***)alocaMatriz(3, nmaq, njob, 3);
		for(int i = 0; i < nmaq; i++)
			for(int j = 0; j < njob; j++)
				for(int k = 0; k < 3; k++)
					this->sol.escalon[i][j][k] = other->sol.escalon[i][j][k];
	}
	exec = prob.exec;
}

JobShop::JobShop(const Problema &prob, int maq, int pos1, int pos2) : Problema::Problema()
{
	JobShop *other = dynamic_cast<JobShop *>(const_cast<Problema *>(&prob));

	this->sol.esc = (short int**)alocaMatriz(2, nmaq, njob, 1);
	for(int i = 0; i < nmaq; i++)
		for(int j = 0; j < njob; j++)
			this->sol.esc[i][j] = other->sol.esc[i][j];

	short int aux = this->sol.esc[maq][pos1];
	this->sol.esc[maq][pos1] = this->sol.esc[maq][pos2];
	this->sol.esc[maq][pos2] = aux;

	this->sol.escalon = NULL;
	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

JobShop::~JobShop()
{
	if(this->sol.esc != NULL)
		desalocaMatriz(2, this->sol.esc, JobShop::nmaq, 1);

	if(this->sol.escalon != NULL)
		desalocaMatriz(3, this->sol.escalon, JobShop::nmaq, JobShop::njob);
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
inline bool JobShop::calcFitness(bool esc)
{
	register int max, cont = 0;
	short int ***aux_esc, **tmp, *pos;
	int ajob, apos, ainic, afim, sum_time;

	pos = (short int*)alocaMatriz(1, nmaq, 1, 1);
	tmp = (short int**)alocaMatriz(2, njob, nmaq+1, 1);
	aux_esc = (short int***)alocaMatriz(3, nmaq, njob, 3);

	register int i, j, k;

	for(i = 0; i < njob; i++)
		for(j = 0; j <= nmaq; j++)
			tmp[i][j] = j == 0 ? 0 : -1;

	for(i = 0; i < nmaq; i++)
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
		{
			ainic = -1;
		}

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
		desalocaMatriz(2, tmp, njob, 1);
		desalocaMatriz(1, pos, 1, 1);

		if(esc == false)
			desalocaMatriz(3, aux_esc, nmaq, njob);
		else
			sol.escalon = aux_esc;

		sol.fitness = sum_time;
		return true;
	}
	else
	{
		desalocaMatriz(3, aux_esc, nmaq, njob);
		desalocaMatriz(2, tmp, njob, 0);
		desalocaMatriz(1, pos, 0, 0);

		sol.fitness = -1;
		return false;
	}
}

bool JobShop::operator == (const Problema& p)
{
	return this->getFitness() == p.getFitness();
}

bool JobShop::operator != (const Problema& p)
{
	return this->getFitness() != p.getFitness();
}

bool JobShop::operator <= (const Problema& p)
{
	return this->getFitness() <= p.getFitness();
}

bool JobShop::operator >= (const Problema& p)
{
	return this->getFitness() >= p.getFitness();
}

bool JobShop::operator < (const Problema& p)
{
	return this->getFitness() < p.getFitness();
}

bool JobShop::operator > (const Problema& p)
{
	return this->getFitness() > p.getFitness();
}

inline void JobShop::imprimir(bool esc)
{
	if(esc == true)
	{
		calcFitness(esc);

		for(int i = 0; i < nmaq; i++)
		{
			printf("maq %d: ", i);
			for(int j = 0; j < njob; j++)
			{
				int k = sol.escalon[i][j][2] - sol.escalon[i][j][1];
				int spc = j == 0 ? sol.escalon[i][j][1] : sol.escalon[i][j][1] - sol.escalon[i][j-1][2];

				while(spc--)
					printf(" ");

				while(k--)
					printf("%c", ((char)sol.escalon[i][j][0]) + 'a');
			}
			printf("\n");
		}
		printf("\n\nLegenda:\n\n");

		for(int i = 0; i < njob; i++)
			printf("%c: job %d\n", ((char)i) + 'a', i);
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

/* Retorna um vizinho aleatorio da solucao atual. */
inline Problema* JobShop::vizinho()
{
	int maq = xRand(rand(), 0, nmaq), p1 = xRand(rand(), 0, njob), p2 = xRand(rand(), 0, njob);
	Problema *job = NULL;

	while(p2 == p1)
		p2 = xRand(rand(), 0, njob);

	job = new JobShop(*this, maq, p1, p2);
	if(job->getFitness() != -1)
	{
		return job;
	}
	else
	{
		delete job;
		return NULL;
	}
}

/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual. */
inline vector<pair<Problema*, InfoTabu*>* >* JobShop::buscaLocal()
{
	if(JobShop::num_vizinhos > MAX_PERMUTACOES)
		return buscaLocal((float)MAX_PERMUTACOES/(float)JobShop::num_vizinhos);

	Problema *job = NULL;
	register int maq, p1, p2;
	int numMaqs = nmaq, numJobs = njob;
	pair<Problema*, InfoTabu*>* temp;
	vector<pair<Problema*, InfoTabu*>* >* local = new vector<pair<Problema*, InfoTabu*>* >();

	for(maq = 0; maq < numMaqs; maq++)
	{
		for(p1 = 0; p1 < numJobs-1; p1++)
		{
			for(p2 = p1+1; p2 < numJobs; p2++)
			{
				job = new JobShop(*this, maq, p1, p2);
				if(job->getFitness() != -1)
				{
					temp = new pair<Problema*, InfoTabu*>();
					temp->first = job;
					temp->second = new InfoTabu_JobShop(maq, p1, p2);

					local->push_back(temp);
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

/* Retorna um conjunto de com uma parcela das solucoes viaveis vizinhas da atual. */
inline vector<pair<Problema*, InfoTabu*>* >* JobShop::buscaLocal(float parcela)
{
	Problema *job = NULL;
	int maq, p1, p2;
	int numMaqs = nmaq, numJobs = njob;
	pair<Problema*, InfoTabu*>* temp;
	vector<pair<Problema*, InfoTabu*>* >* local = new vector<pair<Problema*, InfoTabu*>* >();
	int def;

	def = (int)((float)JobShop::num_vizinhos*parcela);

	if(def > MAX_PERMUTACOES)
		def = MAX_PERMUTACOES;

	for(register int i = 0; i < def; i++)
	{
		maq = xRand(rand(), 0, numMaqs), p1 = xRand(rand(), 0, numJobs), p2 = xRand(rand(), 0, numJobs);

		while(p2 == p1)
			p2 = xRand(rand(), 0, numJobs);

		job = new JobShop(*this, maq, p1, p2);
		if(job->getFitness() != -1)
		{
			temp = new pair<Problema*, InfoTabu*>();
			temp->first = job;
			temp->second = new InfoTabu_JobShop(maq, p1, p2);

			local->push_back(temp);
		}
		else
		{
			delete job;
		}
	}
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Realiza um crossover com uma outra solucao. Usa 2 pivos. */
inline pair<Problema*, Problema*>* JobShop::crossOver(const Problema* parceiro, int tamParticao, int strength)
{
	short int **f1 = (short int**)alocaMatriz(2, nmaq, njob, 1), **f2 = (short int**)alocaMatriz(2, nmaq, njob, 1);
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = tamParticao == 0 ? (JobShop::njob)/2 : tamParticao;
	int numberCrossOver = (int)ceil((float)(strength * nmaq) / 100);
	int inicioPart = 0, fimPart = 0;
	short int *maqs = (short int*)alocaMatriz(1, nmaq, 1, 1);

	JobShop *other = dynamic_cast<JobShop *>(const_cast<Problema *>(parceiro));

	for(register int i = 0; i < nmaq; i++)
		maqs[i] = i;

	random_shuffle(&maqs[0], &maqs[nmaq]);

	for(register int i = 0, j; i < nmaq; i++)
	{
		j = maqs[i];

		if(i < numberCrossOver)
		{
			inicioPart = xRand(rand(), 0, njob);
			fimPart = inicioPart+particao <= njob ? inicioPart+particao : njob;

			swap_vect(this->sol.esc[j], other->sol.esc[j], f1[j], inicioPart, fimPart-inicioPart);
			swap_vect(other->sol.esc[j], this->sol.esc[j], f2[j], inicioPart, fimPart-inicioPart);
		}
		else
		{
			copy(&(this->sol.esc[j][0]), &(this->sol.esc[j][njob]), &(f1[j][0]));
			copy(&(other->sol.esc[j][0]), &(other->sol.esc[j][njob]), &(f2[j][0]));
		}
	}

	desalocaMatriz(1, maqs, nmaq, 1);

	filhos->first = new JobShop(f1);
	filhos->second = new JobShop(f2);

	return filhos;
}

/* Realiza um crossover com uma outra solucao. Usa 1 pivo. */
inline pair<Problema*, Problema*>* JobShop::crossOver(const Problema* parceiro, int strength)
{
	short int **f1 = (short int**)alocaMatriz(2, nmaq, njob, 1), **f2 = (short int**)alocaMatriz(2, nmaq, njob, 1);
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int numberCrossOver = (int)ceil((float)(strength * nmaq) / 100);
	short int *maqs = (short int*)alocaMatriz(1, nmaq, 1, 1);
	int particao = 0;

	JobShop *other = dynamic_cast<JobShop *>(const_cast<Problema *>(parceiro));

	for(register int i = 0; i < nmaq; i++)
		maqs[i] = i;

	random_shuffle(&maqs[0], &maqs[nmaq]);

	for(register int i = 0, j; i < nmaq; i++)
	{
		j = maqs[i];

		if(i < numberCrossOver)
		{
			particao = xRand(rand(), 1, njob);

			swap_vect(this->sol.esc[j], other->sol.esc[j], f1[j], 0, particao);
			swap_vect(other->sol.esc[j], this->sol.esc[j], f2[j], 0, particao);
		}
		else
		{
			copy(&(this->sol.esc[j][0]), &(this->sol.esc[j][njob]), &(f1[j][0]));
			copy(&(other->sol.esc[j][0]), &(other->sol.esc[j][njob]), &(f2[j][0]));
		}
	}

	desalocaMatriz(1, maqs, nmaq, 1);

	filhos->first = new JobShop(f1);
	filhos->second = new JobShop(f2);

	return filhos;
}

/* Devolve uma mutacao aleatoria na solucao atual. */
inline Problema* JobShop::mutacao(int mutMax)
{
	short int **mut = (short int**)alocaMatriz(2, nmaq, njob, 1);
	Problema* vizinho = NULL, *temp = NULL, *mutacao = NULL;

	for(int i = 0; i < nmaq; i++)
		for(int j = 0; j < njob; j++)
			mut[i][j] = sol.esc[i][j];

	temp = new JobShop(mut);
	mutacao = temp;

	while(mutMax-- > 0)
	{
		vizinho = temp->vizinho();

		if(vizinho != NULL)
		{
			delete temp;
			temp = vizinho;
			mutacao = temp;
		}
	}
	return mutacao;
}

inline double JobShop::getFitness() const
{
	return sol.fitness;
}

inline double JobShop::getFitnessMaximize() const
{
	return (double)INV_FITNESS/sol.fitness;
}

inline double JobShop::getFitnessMinimize() const
{
	return (double)sol.fitness;
}

/* Auxiliares */

inline void swap_vect(short int* p1, short int* p2, short int* f, int pos, int tam)
{
	for(register int i = pos; i < pos+tam; i++)
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


inline int findOrdem(int M, int maq, int* job)
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
		for(int i = 0; i < a; i++)
			M[i] = (short int*)malloc(b * sizeof (short int));

		return (void*)M;
	}
	else if(dim == 3)
	{
		short int ***M = (short int***)malloc(a * sizeof (short int**));
		for(int i = 0; i < a; i++)
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
bool fnequal1(Problema* prob1, Problema* prob2)
{
	JobShop *p1 = dynamic_cast<JobShop *>(const_cast<Problema *>(prob1));
	JobShop *p2 = dynamic_cast<JobShop *>(const_cast<Problema *>(prob2));

	if(p1->sol.fitness == p2->sol.fitness)
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
bool fnequal2(Problema* prob1, Problema* prob2)
{
	JobShop *p1 = dynamic_cast<JobShop *>(const_cast<Problema *>(prob1));
	JobShop *p2 = dynamic_cast<JobShop *>(const_cast<Problema *>(prob2));

	return p1->sol.fitness == p2->sol.fitness;
}

// comparator function:
bool fncomp1(Problema *prob1, Problema *prob2)
{
	JobShop *p1 = dynamic_cast<JobShop *>(const_cast<Problema *>(prob1));
	JobShop *p2 = dynamic_cast<JobShop *>(const_cast<Problema *>(prob2));

	if(p1->sol.fitness == p2->sol.fitness)
	{
		for(int i = 0; i < JobShop::nmaq; i++)
			for(int j = 0; j < JobShop::njob; j++)
				if(p1->sol.esc[i][j] != p2->sol.esc[i][j])
					return p1->sol.esc[i][j] < p2->sol.esc[i][j];

		return false;
	}
	else
		return p1->sol.fitness < p2->sol.fitness;
}

// comparator function:
bool fncomp2(Problema *prob1, Problema *prob2)
{
	JobShop *p1 = dynamic_cast<JobShop *>(const_cast<Problema *>(prob1));
	JobShop *p2 = dynamic_cast<JobShop *>(const_cast<Problema *>(prob2));

	return p1->sol.fitness < p2->sol.fitness;
}


inline bool ptcomp(pair<Problema*, InfoTabu*>* p1, pair<Problema*, InfoTabu*>* p2)
{
	return (p1->first->getFitness() > p2->first->getFitness());
}
