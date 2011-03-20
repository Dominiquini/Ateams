#include "KnapSack.hpp"

using namespace std;

/* Static Members */

ProblemType Problema::TIPO = MAXIMIZACAO;

double Problema::best = 0;
double Problema::worst = 0;
int Problema::numInst = 0;
long int Problema::totalNumInst = 0;

char KnapSack::name[128];
double *KnapSack::values = NULL, **KnapSack::constraint = NULL, *KnapSack::limit = NULL;
int KnapSack::nitens = 0, KnapSack::ncontraint = 0;

int KnapSack::num_vizinhos = 0;

Problema* Problema::randSoluction()
{
	return new KnapSack();
}

Problema* Problema::copySoluction(const Problema& prob)
{
	return new KnapSack(prob);
}


void Problema::leProblema(FILE *f)
{
	if(!fgets (KnapSack::name, 128, f))
		exit(1);

	if(!fscanf (f, "%d %d %*d\n", &KnapSack::nitens, &KnapSack::ncontraint))
		exit(1);

	Problema::alocaMemoria();

	for(int i = 0; i < KnapSack::nitens; i++)
	{
		if (!fscanf (f, "%lf", &KnapSack::values[i]))
			exit(1);
	}

	double **constraints = (double**)malloc(KnapSack::ncontraint * sizeof(double*));
	for(int i = 0; i < KnapSack::ncontraint; i++)
		constraints[i] = (double*)malloc(KnapSack::nitens * sizeof(double));

	for(int i = 0; i < KnapSack::ncontraint; i++)
	{
		for (int j = 0; j < KnapSack::nitens; j++)
		{
			if (!fscanf (f, "%lf", &constraints[i][j]))
				exit(1);
		}
	}

	for(int i = 0; i < KnapSack::nitens; i++)
		for(int j = 0; j < KnapSack::ncontraint; j++)
			KnapSack::constraint[i][j] = constraints[j][i];

	for(int i = 0; i < KnapSack::ncontraint; i++)
		free(constraints[i]);
	free(constraints);

	for(int i = 0; i < KnapSack::ncontraint; i++)
	{
		if (!fscanf (f, "%lf", &KnapSack::limit[i]))
			exit(1);
	}

	for(int i = 1; i < KnapSack::nitens + 2; i++)
		KnapSack::num_vizinhos += i;

	return;
}

list<Problema*>* Problema::lePopulacao(char *log)
{
	FILE *f = fopen(log, "r");

	if(f != NULL)
	{
		list<Problema*>* popInicial = new list<Problema*>();
		int npop, nitens, nconstraint, valorTotal, limit;
		char format_type[32];
		short int *prob;
		Problema* p;

		if(!fscanf (f, "%d %d %d %s\n", &npop, &nitens, &nconstraint, format_type))
		{
			printf("Arquivo de Log Incorreto - Cabeçallho!\n\n");
			exit(1);
		}

		if(nitens != KnapSack::nitens || nconstraint != KnapSack::ncontraint)
		{
			printf("Arquivo de Log Incorreto - Problemas Diferentes!\n\n");
			exit(1);
		}

		for(int s = 0; s < npop; s++)
		{
			prob = (short int*)malloc(nitens * sizeof(short int));

			if(!fscanf (f, "%d\n", &valorTotal))
			{
				printf("Arquivo de Log Incorreto - Fitness!\n\n");
				exit(1);
			}

			if(!fscanf (f, "%d ", &limit))
			{
				printf("Arquivo de Log Incorreto - Limite!\n\n");
				exit(1);
			}

			for(int i = 0; i < nitens; i++)
			{
				if(!fscanf (f, "%hd ", &prob[i]))
				{
					printf("Arquivo de Log Incorreto - Soluçao!\n\n");
					exit(1);
				}
			}

			p = new KnapSack(prob, limit);
			if(valorTotal != p->getFitness())
			{
				printf("Arquivo de Log Incorreto - Fitness Diferentes!\n\n");
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
	short int *prob;

	fprintf(f, "%d %d %d %s\n\n", sizePop, KnapSack::nitens, KnapSack::ncontraint, "src_1");

	for(iter = popInicial->begin(); iter != popInicial->end(); iter++)
	{
		prob = dynamic_cast<KnapSack *>(*iter)->getSoluction().ordemItens;

		fprintf(f, "%d\n", (int)dynamic_cast<KnapSack *>(*iter)->getSoluction().fitness);
		fprintf(f, "%d ", (int)dynamic_cast<KnapSack *>(*iter)->getSoluction().limit);
		for(int i = 0; i < KnapSack::nitens; i++)
		{
			fprintf(f, "%d ", prob[i]);
		}
		fprintf(f, "\n\n");
	}
	fclose(f);
}

void Problema::escreveResultado(char *dados, char *parametros, ExecInfo *info, char *resultado)
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
	KnapSack::values = (double*)malloc(KnapSack::nitens * sizeof(double));

	KnapSack::limit = (double*)malloc(KnapSack::ncontraint * sizeof(double));

	KnapSack::constraint = (double**)malloc(KnapSack::nitens * sizeof(double*));
	for(int i = 0; i < KnapSack::nitens; i++)
		KnapSack::constraint[i] = (double*)malloc(KnapSack::ncontraint * sizeof(double));
}

void Problema::desalocaMemoria()
{
	free(KnapSack::values);

	free(KnapSack::limit);

	for(int i = 0; i < KnapSack::ncontraint; i++)
		free(KnapSack::constraint[i]);
	free(KnapSack::constraint);
}


/* Metodos */

KnapSack::KnapSack() : Problema::Problema()
{
	sol.ordemItens = (short int*)malloc(nitens * sizeof(short int));

	for(register int i = 0; i < nitens; i++)
		sol.ordemItens[i] = i;

	random_shuffle(&sol.ordemItens[0], &sol.ordemItens[nitens]);

	sol.limit = -1;

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

KnapSack::KnapSack(short int *prob) : Problema::Problema()
{
	sol.ordemItens = prob;
	sol.limit = -1;

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

KnapSack::KnapSack(short int *prob, int limit) : Problema::Problema()
{
	sol.ordemItens = prob;
	sol.limit = limit;

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

KnapSack::KnapSack(const Problema &prob) : Problema::Problema()
{
	KnapSack *other = dynamic_cast<KnapSack *>(const_cast<Problema *>(&prob));

	this->sol.ordemItens = (short int*)malloc(nitens * sizeof(short int));
	for(int i = 0; i < nitens; i++)
		this->sol.ordemItens[i] = other->sol.ordemItens[i];

	this->sol.limit = other->sol.limit;

	this->sol.fitness = other->sol.fitness;
	exec = prob.exec;
}

KnapSack::KnapSack(const Problema &prob, int pos1, int pos2) : Problema::Problema()
{
	KnapSack *other = dynamic_cast<KnapSack *>(const_cast<Problema *>(&prob));

	this->sol.ordemItens = (short int*)malloc(nitens * sizeof(short int));
	for(int i = 0; i < nitens; i++)
		this->sol.ordemItens[i] = other->sol.ordemItens[i];

	short int aux = this->sol.ordemItens[pos1];
	this->sol.ordemItens[pos1] = this->sol.ordemItens[pos2];
	this->sol.ordemItens[pos2] = aux;

	sol.limit = -1;

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

KnapSack::~KnapSack()
{
	if(this->sol.ordemItens != NULL)
		free(this->sol.ordemItens);
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
inline bool KnapSack::calcFitness(bool esc)
{
	if(sol.limit == -1)
	{
		vector<double> tempConstraints(ncontraint, 0);

		register double fitness = 0;
		register int item = 0, limit = 0;
		for(limit = 0; limit < nitens; limit++)
		{
			item = sol.ordemItens[limit];

			if(!constraintVerify(item, tempConstraints))
				break;

			fitness += values[item];
		}

		sort(&sol.ordemItens[0], &sol.ordemItens[limit]);
		sort(&sol.ordemItens[limit], &sol.ordemItens[nitens]);

		sol.fitness = fitness;
		sol.limit = limit;
	}
	else
	{
		sol.fitness = 0;
		for(register int i = 0; i < sol.limit; i++)
			sol.fitness += values[sol.ordemItens[i]];
	}

	return true;
}

inline void KnapSack::imprimir(bool esc)
{
	if(esc == true)
	{
		double sumItens = 0;
		int numItens = 0;

		printf("+sack: ");
		for(int i = 0; i < sol.limit; i++)
		{
			printf("|%.4d|", sol.ordemItens[i]+1);
			sumItens += values[sol.ordemItens[i]];
			numItens++;
		}

		printf(" ==> %0.2f (%d)\n", sumItens, numItens);

		sumItens = 0;
		numItens = 0;

		printf("-sack: ");
		for(int i = sol.limit; i < nitens; i++)
		{
			printf("|%.4d|", sol.ordemItens[i]+1);
			sumItens += values[sol.ordemItens[i]];
			numItens++;
		}

		printf(" ==> %0.2f (%d)\n", sumItens, numItens);
	}
	else
	{
		for(int i = 0; i < nitens; i++)
			printf("%d ", sol.ordemItens[i]);
	}
	printf("\n");
}

/* Retorna um vizinho aleatorio da solucao atual. */
inline Problema* KnapSack::vizinho()
{
	int p1 = xRand(rand(), 0, sol.limit), p2 = xRand(rand(), sol.limit, nitens);
	Problema *prob = NULL;

	prob = new KnapSack(*this, p1, p2);

	return prob;
}

/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual. */
inline vector<pair<Problema*, InfoTabu*>* >* KnapSack::buscaLocal()
{
	if(KnapSack::num_vizinhos > MAX_PERMUTACOES)
		return buscaLocal((float)MAX_PERMUTACOES/(float)KnapSack::num_vizinhos);

	register int p1, p2;
	Problema *prob = NULL;
	pair<Problema*, InfoTabu*>* temp;
	vector<pair<Problema*, InfoTabu*>* >* local = new vector<pair<Problema*, InfoTabu*>* >();

	for(p1 = 0; p1 < sol.limit; p1++)
	{
		for(p2 = sol.limit; p2 < nitens; p2++)
		{
			prob = new KnapSack(*this, p1, p2);

			temp = new pair<Problema*, InfoTabu*>();
			temp->first = prob;
			temp->second = new InfoTabu_KnapSack(p1, p2);

			local->push_back(temp);
		}
	}

	random_shuffle(local->begin(), local->end());
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Retorna um conjunto de com uma parcela das solucoes viaveis vizinhas da atual. */
inline vector<pair<Problema*, InfoTabu*>* >* KnapSack::buscaLocal(float parcela)
{
	Problema *prob = NULL;
	int p1, p2;
	int numItens = nitens;
	pair<Problema*, InfoTabu*>* temp;
	vector<pair<Problema*, InfoTabu*>* >* local = new vector<pair<Problema*, InfoTabu*>* >();
	int def;

	def = (int)((float)KnapSack::num_vizinhos*parcela);

	if(def > MAX_PERMUTACOES)
		def = MAX_PERMUTACOES;

	for(register int i = 0; i < def; i++)
	{
		p1 = xRand(rand(), 0, sol.limit), p2 = xRand(rand(), sol.limit, numItens);

		prob = new KnapSack(*this, p1, p2);

		temp = new pair<Problema*, InfoTabu*>();
		temp->first = prob;
		temp->second = new InfoTabu_KnapSack(p1, p2);

		local->push_back(temp);
	}
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Realiza um crossover com uma outra solucao. Usa 2 pivos. */
inline pair<Problema*, Problema*>* KnapSack::crossOver(const Problema* parceiro, int tamParticao, int strength)
{
	short int *f1 = (short int*)malloc(nitens * sizeof(short int)), *f2 = (short int*)malloc(nitens * sizeof(short int));
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = tamParticao == 0 ? (nitens)/2 : tamParticao;
	int inicioPart = 0, fimPart = 0;

	KnapSack *other = dynamic_cast<KnapSack *>(const_cast<Problema *>(parceiro));

	inicioPart = xRand(rand(), 0, nitens);
	fimPart = inicioPart+particao <= nitens ? inicioPart+particao : nitens;

	swap_vect(this->sol.ordemItens, other->sol.ordemItens, f1, inicioPart, fimPart-inicioPart);
	swap_vect(other->sol.ordemItens, this->sol.ordemItens, f2, inicioPart, fimPart-inicioPart);

	filhos->first = new KnapSack(f1);
	filhos->second = new KnapSack(f2);

	return filhos;
}

/* Realiza um crossover com uma outra solucao. Usa 1 pivo. */
inline pair<Problema*, Problema*>* KnapSack::crossOver(const Problema* parceiro, int strength)
{
	short int *f1 = (short int*)malloc(nitens * sizeof(short int)), *f2 = (short int*)malloc(nitens * sizeof(short int));
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = 0;

	KnapSack *other = dynamic_cast<KnapSack *>(const_cast<Problema *>(parceiro));

	particao = xRand(rand(), 1, nitens);

	swap_vect(this->sol.ordemItens, other->sol.ordemItens, f1, 0, particao);
	swap_vect(other->sol.ordemItens, this->sol.ordemItens, f2, 0, particao);

	filhos->first = new KnapSack(f1);
	filhos->second = new KnapSack(f2);

	return filhos;
}

/* Devolve uma mutacao aleatoria na solucao atual. */
inline Problema* KnapSack::mutacao(int mutMax)
{
	short int *mut = (short int*)malloc(nitens * sizeof(short int));
	Problema* vizinho = NULL, *temp = NULL, *mutacao = NULL;

	for(int i = 0; i < nitens; i++)
		mut[i] = sol.ordemItens[i];

	temp = new KnapSack(mut);
	mutacao = temp;

	while(mutMax-- > 0)
	{
		vizinho = temp->vizinho();

		delete temp;
		temp = vizinho;
		mutacao = temp;
	}

	return mutacao;
}

inline double KnapSack::getFitness() const
{
	return sol.fitness;
}

inline double KnapSack::getFitnessMaximize() const
{
	return (double)sol.fitness;
}

inline double KnapSack::getFitnessMinimize() const
{
	return (double)INV_FITNESS/sol.fitness;
}

/* Auxiliares */

inline void swap_vect(short int* p1, short int* p2, short int* f, int pos, int tam)
{
	for(register int i = pos; i < pos+tam; i++)
		f[i] = p1[i];

	for(register int i = 0, j = 0; i < KnapSack::nitens && j < KnapSack::nitens; i++)
	{
		if(j == pos)
			j = pos+tam;

		if(find(&p1[pos], &p1[pos+tam], p2[i]) == &p1[pos+tam])
			f[j++] = p2[i];
	}
	return;
}


inline bool constraintVerify(int item, vector<double> &constraints)
{
	vector<double>::iterator constraint;
	register int c = 0;

	for(constraint = constraints.begin(); constraint != constraints.end(); constraint++)
	{
		*constraint += KnapSack::constraint[item][c];

		if(*constraint > KnapSack::limit[c++])
			return false;
	}

	return true;
}

// comparator function:
bool fnequal1(Problema* prob1, Problema* prob2)
{
	KnapSack *p1 = dynamic_cast<KnapSack *>(const_cast<Problema *>(prob1));
	KnapSack *p2 = dynamic_cast<KnapSack *>(const_cast<Problema *>(prob2));

	if(p1->sol.fitness == p2->sol.fitness)
	{
		for(int i = 0; i < KnapSack::nitens; i++)
			if(p1->sol.ordemItens[i] != p2->sol.ordemItens[i])
				return false;

		return true;
	}
	else
		return false;
}

// comparator function:
bool fnequal2(Problema* prob1, Problema* prob2)
{
	KnapSack *p1 = dynamic_cast<KnapSack *>(const_cast<Problema *>(prob1));
	KnapSack *p2 = dynamic_cast<KnapSack *>(const_cast<Problema *>(prob2));

	return p1->sol.fitness == p2->sol.fitness;
}

// comparator function:
bool fncomp1(Problema *prob1, Problema *prob2)
{
	KnapSack *p1 = dynamic_cast<KnapSack *>(const_cast<Problema *>(prob1));
	KnapSack *p2 = dynamic_cast<KnapSack *>(const_cast<Problema *>(prob2));

	if(p1->sol.fitness == p2->sol.fitness)
	{
		for(int i = 0; i < KnapSack::nitens; i++)
			if(p1->sol.ordemItens[i] != p2->sol.ordemItens[i])
				return p1->sol.ordemItens[i] > p2->sol.ordemItens[i];

		return false;
	}
	else
		return p1->sol.fitness > p2->sol.fitness;
}

// comparator function:
bool fncomp2(Problema *prob1, Problema *prob2)
{
	KnapSack *p1 = dynamic_cast<KnapSack *>(const_cast<Problema *>(prob1));
	KnapSack *p2 = dynamic_cast<KnapSack *>(const_cast<Problema *>(prob2));

	return p1->sol.fitness > p2->sol.fitness;
}


inline bool ptcomp(pair<Problema*, InfoTabu*>* p1, pair<Problema*, InfoTabu*>* p2)
{
	return (p1->first->getFitness() < p2->first->getFitness());
}
