#include "KnapSack.h"

using namespace std;

/* Static Members */

ProblemType Problema::TIPO = MAXIMIZACAO;

double Problema::best = 0;
double Problema::worst = 0;
int Problema::numInst = 0;
long int Problema::totalNumInst = 0;

char KnapSack_2::name[128];
double *KnapSack_2::values = NULL, **KnapSack_2::constraint = NULL, *KnapSack_2::limit = NULL;
int KnapSack_2::nitens = 0, KnapSack_2::ncontraint = 0;

int KnapSack_2::num_vizinhos = 0;

Problema* Problema::randSoluction()
{
	return new KnapSack_2();
}

Problema* Problema::copySoluction(const Problema& prob)
{
	return new KnapSack_2(prob);
}


void Problema::leProblema(FILE *f)
{
	if(!fgets (KnapSack_2::name, 128, f))
		exit(1);

	if(!fscanf (f, "%d %d %*d\n", &KnapSack_2::nitens, &KnapSack_2::ncontraint))
		exit(1);

	Problema::alocaMemoria();

	for(int i = 0; i < KnapSack_2::nitens; i++)
	{
		if (!fscanf (f, "%lf", &KnapSack_2::values[i]))
			exit(1);
	}

	for(int i = 0; i < KnapSack_2::ncontraint; i++)
	{
		for (int j = 0; j < KnapSack_2::nitens; j++)
		{
			if (!fscanf (f, "%lf", &KnapSack_2::constraint[i][j]))
				exit(1);
		}
	}

	for(int i = 0; i < KnapSack_2::ncontraint; i++)
	{
		if (!fscanf (f, "%lf", &KnapSack_2::limit[i]))
			exit(1);
	}

	for(int i = 1; i < KnapSack_2::nitens + 2; i++)
		KnapSack_2::num_vizinhos += i;

	return;
}

list<Problema*>* Problema::lePopulacao(char *log)
{
	FILE *f = fopen(log, "r");

	if(f != NULL)
	{
		list<Problema*>* popInicial = new list<Problema*>();
		int npop, nitens, nconstraint, valorTotal;
		char format_type[32];
		short int *prob;
		Problema* p;

		if(!fscanf (f, "%d %d %d %s\n", &npop, &nitens, &nconstraint, format_type))
		{
			printf("Arquivo de Log Incorreto - Cabeçalho!\n\n");
			exit(1);
		}

		if(nitens != KnapSack_2::nitens || nconstraint != KnapSack_2::ncontraint)
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

			if(!strcmp(format_type, "src_2"))
			{
				for(int i = 0; i < nitens; i++)
				{
					if(!fscanf (f, "%hd", &prob[i]))
					{
						printf("Arquivo de Log Incorreto - Solução!\n\n");
						exit(1);
					}
				}
			}
			else
			{
				int max = 0, pos = 0;

				if(!fscanf (f, "%d", &max))
				{
					printf("Arquivo de Log Incorreto - Separador!\n\n");
					exit(1);
				}

				for(int i = 0; i < nitens; i++)
				{
					if(!fscanf (f, "%d", &pos))
					{
						printf("Arquivo de Log Incorreto - Solução!\n\n");
						exit(1);
					}

					if(i < max)
						prob[pos] = 1;
					else
						prob[pos] = 0;
				}
			}

			p = new KnapSack_2(prob);

			if(valorTotal != p->getFitness())
			{
				printf("Arquivo de Log Incorreto - Fitness Diferente!\n\n");
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

	fprintf(f, "%d %d %d %s\n\n", sizePop, KnapSack_2::nitens, KnapSack_2::ncontraint, "src_2");

	for(iter = popInicial->begin(); iter != popInicial->end(); iter++)
	{
		prob = dynamic_cast<KnapSack_2 *>(*iter)->getSoluction().itens;

		fprintf(f, "%d\n", (int)dynamic_cast<KnapSack_2 *>(*iter)->getSoluction().fitness);

		for(int i = 0; i < KnapSack_2::nitens; i++)
		{
			fprintf(f, "%d ", prob[i]);
		}
		fprintf(f, "\n\n");
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
	KnapSack_2::values = (double*)malloc(KnapSack_2::nitens * sizeof(double));

	KnapSack_2::limit = (double*)malloc(KnapSack_2::ncontraint * sizeof(double));

	KnapSack_2::constraint = (double**)malloc(KnapSack_2::ncontraint * sizeof(double*));
	for(int i = 0; i < KnapSack_2::ncontraint; i++)
		KnapSack_2::constraint[i] = (double*)malloc(KnapSack_2::nitens * sizeof(double));
}

void Problema::desalocaMemoria()
{
	free(KnapSack_2::values);

	free(KnapSack_2::limit);

	for(int i = 0; i < KnapSack_2::ncontraint; i++)
		free(KnapSack_2::constraint[i]);
	free(KnapSack_2::constraint);
}

double Problema::melhora(double oldP, double newP)
{
	return newP - oldP;
}

double Problema::melhora(Problema& oldP, Problema& newP)
{
	return newP.getFitness() - oldP.getFitness();
}

/* Metodos */

KnapSack_2::KnapSack_2() : Problema::Problema()
{
	sol.itens = (short int*)malloc(nitens * sizeof(short int));
	short int *retirar = (short int*)malloc(nitens * sizeof(short int));
	register int pos = 0;

	for(register int i = 0; i < nitens; i++)
	{
		sol.itens[i] = 1;
		retirar[i] = i;
	}

	random_shuffle(&retirar[0], &retirar[nitens]);

	while(!calcFitness(false) && pos < nitens)
	{
		sol.itens[retirar[pos++]] = 0;
	}

	free(retirar);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}


KnapSack_2::KnapSack_2(short int *prob) : Problema::Problema()
{
	sol.itens = prob;

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

KnapSack_2::KnapSack_2(const Problema &prob) : Problema::Problema()
{
	KnapSack_2 *other = dynamic_cast<KnapSack_2 *>(const_cast<Problema *>(&prob));

	this->sol.itens = (short int*)malloc(nitens * sizeof(short int));
	for(int i = 0; i < nitens; i++)
		this->sol.itens[i] = other->sol.itens[i];

	this->sol.fitness = other->sol.fitness;
	exec = prob.exec;
}

KnapSack_2::KnapSack_2(const Problema &prob, int pos1, int pos2) : Problema::Problema()
{
	KnapSack_2 *other = dynamic_cast<KnapSack_2 *>(const_cast<Problema *>(&prob));

	this->sol.itens = (short int*)malloc(nitens * sizeof(short int));
	for(int i = 0; i < nitens; i++)
		this->sol.itens[i] = other->sol.itens[i];

	if(pos2 == nitens)
		this->sol.itens[pos1] = 0;
	else if(pos2 == nitens+1)
		this->sol.itens[pos1] = 1;
	else
	{
		short int aux = this->sol.itens[pos1];
		this->sol.itens[pos1] = this->sol.itens[pos2];
		this->sol.itens[pos2] = aux;
	}

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

KnapSack_2::~KnapSack_2()
{
	if(this->sol.itens != NULL)
		free(this->sol.itens);
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
inline bool KnapSack_2::calcFitness(bool esc)
{
	sol.fitness = -1;
	for(register int c = 0; c < KnapSack_2::ncontraint; c++)
		if(!constraintVerify(c, sol.itens))
			return false;

	sol.fitness = 0;
	for(register int i = 0; i < KnapSack_2::nitens; i++)
		if(sol.itens[i] == 1)
			sol.fitness += KnapSack_2::values[i];

	return true;
}

bool KnapSack_2::operator == (const Problema& p)
{
	return this->getFitness() == p.getFitness();
}

bool KnapSack_2::operator != (const Problema& p)
{
	return this->getFitness() != p.getFitness();
}

bool KnapSack_2::operator <= (const Problema& p)
{
	return this->getFitness() <= p.getFitness();
}

bool KnapSack_2::operator >= (const Problema& p)
{
	return this->getFitness() >= p.getFitness();
}

bool KnapSack_2::operator < (const Problema& p)
{
	return this->getFitness() < p.getFitness();
}

bool KnapSack_2::operator > (const Problema& p)
{
	return this->getFitness() > p.getFitness();
}

inline void KnapSack_2::imprimir(bool esc)
{
	if(esc == true)
	{
		printf("sack: ");
		for(int i = 0; i < nitens; i++)
			if(sol.itens[i] == 1)
				printf("|%.3d|", i);

		printf(" ==> %0.2f\n", sol.fitness);
	}
	else
	{
		for(int i = 0; i < nitens; i++)
			printf("%d ", sol.itens[i]);
	}
	printf("\n");
}

/* Retorna um vizinho aleatorio da solucao atual. */
inline Problema* KnapSack_2::vizinho()
{
	int p1 = xRand(rand(), 0, nitens), p2 = xRand(rand(), 0, nitens + 2);
	Problema *prob = NULL;

	while((p2 == p1) || (p2 < nitens && sol.itens[p1] == sol.itens[p2]) || (p2 == nitens && sol.itens[p1] == 0) || (p2 == nitens + 1 && sol.itens[p1] == 1))
		p2 = xRand(rand(), 0, nitens + 2);

	prob = new KnapSack_2(*this, p1, p2);
	if(prob->getFitness() != -1)
	{
		return prob;
	}
	else
	{
		delete prob;
		return NULL;
	}
}

/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual. */
inline vector<pair<Problema*, InfoTabu*>* >* KnapSack_2::buscaLocal()
{
	if(KnapSack_2::num_vizinhos > MAX_PERMUTACOES)
		return buscaLocal((float)MAX_PERMUTACOES/(float)KnapSack_2::num_vizinhos);

	register int p1, p2;
	Problema *prob = NULL;
	pair<Problema*, InfoTabu*>* temp;
	vector<pair<Problema*, InfoTabu*>* >* local = new vector<pair<Problema*, InfoTabu*>* >();

	for(p1 = 0; p1 < nitens; p1++)
	{
		for(p2 = p1+1; p2 < nitens+2; p2++)
		{
			if((p2 < nitens && sol.itens[p1] != sol.itens[p2]) || (p2 == nitens && sol.itens[p1] != 0) || (p2 == nitens+1 && sol.itens[p1] != 1))
			{
				prob = new KnapSack_2(*this, p1, p2);
				if(prob->getFitness() != -1)
				{
					temp = new pair<Problema*, InfoTabu*>();
					temp->first = prob;
					temp->second = new InfoTabu_KnapSack_2(p1, p2);

					local->push_back(temp);
				}
				else
				{
					delete prob;
				}
			}
		}
	}
	random_shuffle(local->begin(), local->end());
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Retorna um conjunto de com uma parcela das solucoes viaveis vizinhas da atual. */
inline vector<pair<Problema*, InfoTabu*>* >* KnapSack_2::buscaLocal(float parcela)
{
	Problema *prob = NULL;
	int p1, p2;
	int numItens = nitens;
	pair<Problema*, InfoTabu*>* temp;
	vector<pair<Problema*, InfoTabu*>* >* local = new vector<pair<Problema*, InfoTabu*>* >();
	int def;

	def = (int)((float)KnapSack_2::num_vizinhos*parcela);

	if(def > MAX_PERMUTACOES)
		def = MAX_PERMUTACOES;

	for(register int i = 0; i < def; i++)
	{
		p1 = xRand(rand(), 0, numItens), p2 = xRand(rand(), 0, numItens + 2);

		while((p2 == p1) || (p2 < nitens && sol.itens[p1] == sol.itens[p2]) || (p2 == nitens && sol.itens[p1] == 0) || (p2 == nitens + 1 && sol.itens[p1] == 1))
			p2 = xRand(rand(), 0, numItens + 2);

		prob = new KnapSack_2(*this, p1, p2);
		if(prob->getFitness() != -1)
		{
			temp = new pair<Problema*, InfoTabu*>();
			temp->first = prob;
			temp->second = new InfoTabu_KnapSack_2(p1, p2);

			local->push_back(temp);
		}
		else
		{
			delete prob;
		}
	}
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Realiza um crossover com uma outra solucao. Usa 2 pivos. */
inline pair<Problema*, Problema*>* KnapSack_2::crossOver(const Problema* parceiro, int tamParticao, int strength)
{
	short int *f1 = (short int*)malloc(nitens * sizeof(short int)), *f2 = (short int*)malloc(nitens * sizeof(short int));
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = tamParticao == 0 ? (KnapSack_2::nitens)/2 : tamParticao;
	int inicioPart = 0, fimPart = 0;

	KnapSack_2 *other = dynamic_cast<KnapSack_2 *>(const_cast<Problema *>(parceiro));

	inicioPart = xRand(rand(), 0, nitens);
	fimPart = inicioPart+particao <= nitens ? inicioPart+particao : nitens;

	swap_vect(this->sol.itens, other->sol.itens, f1, inicioPart, fimPart-inicioPart);
	swap_vect(other->sol.itens, this->sol.itens, f2, inicioPart, fimPart-inicioPart);

	filhos->first = new KnapSack_2(f1);
	filhos->second = new KnapSack_2(f2);

	return filhos;
}

/* Realiza um crossover com uma outra solucao. Usa 1 pivo. */
inline pair<Problema*, Problema*>* KnapSack_2::crossOver(const Problema* parceiro, int strength)
{
	short int *f1 = (short int*)malloc(nitens * sizeof(short int)), *f2 = (short int*)malloc(nitens * sizeof(short int));
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = 0;

	KnapSack_2 *other = dynamic_cast<KnapSack_2 *>(const_cast<Problema *>(parceiro));

	particao = xRand(rand(), 1, nitens);

	swap_vect(this->sol.itens, other->sol.itens, f1, 0, particao);
	swap_vect(other->sol.itens, this->sol.itens, f2, 0, particao);

	filhos->first = new KnapSack_2(f1);
	filhos->second = new KnapSack_2(f2);

	return filhos;
}

/* Devolve uma mutacao aleatoria na solucao atual. */
inline Problema* KnapSack_2::mutacao(int mutMax)
{
	short int *mut = (short int*)malloc(nitens * sizeof(short int));
	Problema* vizinho = NULL, *temp = NULL, *mutacao = NULL;

	for(int i = 0; i < nitens; i++)
		mut[i] = sol.itens[i];

	temp = new KnapSack_2(mut);
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

inline double KnapSack_2::getFitness() const
{
	return sol.fitness;
}

inline double KnapSack_2::getFitnessMaximize() const
{
	return (double)sol.fitness;
}

inline double KnapSack_2::getFitnessMinimize() const
{
	return (double)INV_FITNESS/sol.fitness;
}

/* Auxiliares */

inline void swap_vect(short int* p1, short int* p2, short int* filho, int posInicio, int posFim)
{
	for(register int i = 0; i < KnapSack_2::nitens; i++)
		if(i >= posInicio && i < posFim)
			filho[i] = p2[i];
		else
			filho[i] = p1[i];

	return;
}


inline bool constraintVerify(int c, short int* itens)
{
	double sumC = 0;

	for(register int i = 0; i < KnapSack_2::nitens; i++)
		if(itens[i] == 1)
			sumC += KnapSack_2::constraint[c][i];

	return sumC <= KnapSack_2::limit[c];
}

// comparator function:
bool fnequal1(Problema* prob1, Problema* prob2)
{
	KnapSack_2 *p1 = dynamic_cast<KnapSack_2 *>(const_cast<Problema *>(prob1));
	KnapSack_2 *p2 = dynamic_cast<KnapSack_2 *>(const_cast<Problema *>(prob2));

	if(p1->sol.fitness == p2->sol.fitness)
	{
		for(int i = 0; i < KnapSack_2::nitens; i++)
			if(p1->sol.itens[i] != p2->sol.itens[i])
				return false;

		return true;
	}
	else
		return false;
}

// comparator function:
bool fnequal2(Problema* prob1, Problema* prob2)
{
	KnapSack_2 *p1 = dynamic_cast<KnapSack_2 *>(const_cast<Problema *>(prob1));
	KnapSack_2 *p2 = dynamic_cast<KnapSack_2 *>(const_cast<Problema *>(prob2));

	return p1->sol.fitness == p2->sol.fitness;
}

// comparator function:
bool fncomp1(Problema *prob1, Problema *prob2)
{
	KnapSack_2 *p1 = dynamic_cast<KnapSack_2 *>(const_cast<Problema *>(prob1));
	KnapSack_2 *p2 = dynamic_cast<KnapSack_2 *>(const_cast<Problema *>(prob2));

	if(p1->sol.fitness == p2->sol.fitness)
	{
		for(int i = 0; i < KnapSack_2::nitens; i++)
			if(p1->sol.itens[i] != p2->sol.itens[i])
				return p1->sol.itens[i] > p2->sol.itens[i];

		return false;
	}
	else
		return p1->sol.fitness > p2->sol.fitness;
}

// comparator function:
bool fncomp2(Problema *prob1, Problema *prob2)
{
	KnapSack_2 *p1 = dynamic_cast<KnapSack_2 *>(const_cast<Problema *>(prob1));
	KnapSack_2 *p2 = dynamic_cast<KnapSack_2 *>(const_cast<Problema *>(prob2));

	return p1->sol.fitness > p2->sol.fitness;
}


inline bool ptcomp(pair<Problema*, InfoTabu*>* p1, pair<Problema*, InfoTabu*>* p2)
{
	return (p1->first->getFitness() < p2->first->getFitness());
}
