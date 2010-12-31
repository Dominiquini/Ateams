#include "KnapSack.h"

using namespace std;

/* Static Members */

ProblemType Problema::TIPO = MAXIMIZACAO;

double Problema::best = 0;
double Problema::worst = 0;
int Problema::numInst = 0;
long int Problema::totalNumInst = 0;

char KnapSack_1::name[128];
double *KnapSack_1::values = NULL, **KnapSack_1::constraint = NULL, *KnapSack_1::limit = NULL;
int KnapSack_1::nitens = 0, KnapSack_1::ncontraint = 0;

int KnapSack_1::num_vizinhos = 0;

Problema* Problema::randSoluction()
{
	return new KnapSack_1();
}

Problema* Problema::copySoluction(const Problema& prob)
{
	return new KnapSack_1(prob);
}


void Problema::leProblema(FILE *f)
{
	if(!fgets (KnapSack_1::name, 128, f))
		exit(1);

	if(!fscanf (f, "%d %d %*d\n", &KnapSack_1::nitens, &KnapSack_1::ncontraint))
		exit(1);

	Problema::alocaMemoria();

	for(int i = 0; i < KnapSack_1::nitens; i++)
	{
		if (!fscanf (f, "%lf", &KnapSack_1::values[i]))
			exit(1);
	}

	for(int i = 0; i < KnapSack_1::ncontraint; i++)
	{
		for (int j = 0; j < KnapSack_1::nitens; j++)
		{
			if (!fscanf (f, "%lf", &KnapSack_1::constraint[i][j]))
				exit(1);
		}
	}

	for(int i = 0; i < KnapSack_1::ncontraint; i++)
	{
		if (!fscanf (f, "%lf", &KnapSack_1::limit[i]))
			exit(1);
	}

	for(int i = 1; i < KnapSack_1::nitens + 2; i++)
		KnapSack_1::num_vizinhos += i;

	return;
}

list<Problema*>* Problema::lePopulacao(char *log)
{
	FILE *f = fopen(log, "r");

	if(f != NULL)
	{
		list<Problema*>* popInicial = new list<Problema*>();
		int npop, nitens, nconstraint, valorTotal, max;
		char format_type[32];
		short int *prob;
		Problema* p;

		if(!fscanf (f, "%d %d %d %s\n", &npop, &nitens, &nconstraint, format_type))
		{
			printf("Arquivo de Log Incorreto - Cabeçallho!\n\n");
			exit(1);
		}

		if(nitens != KnapSack_1::nitens || nconstraint != KnapSack_1::ncontraint)
		{
			printf("Arquivo de Log Incorreto - Problemas Diferentes!\n\n");
			exit(1);
		}

		if(!strcmp(format_type, "src_1"))
		{
			for(int s = 0; s < npop; s++)
			{
				prob = (short int*)malloc(nitens * sizeof(short int));

				if(!fscanf (f, "%d\n", &valorTotal))
				{
					printf("Arquivo de Log Incorreto - Fitness!\n\n");
					exit(1);
				}

				if(!fscanf (f, "%d", &max))
				{
					printf("Arquivo de Log Incorreto - Separador!\n\n");
					exit(1);
				}

				for(int i = 0; i < nitens; i++)
				{
					if(!fscanf (f, "%hd", &prob[i]))
					{
						printf("Arquivo de Log Incorreto - Soluçao!\n\n");
						exit(1);
					}
				}

				p = new KnapSack_1(prob, max);
				if(valorTotal != p->getFitness())
				{
					printf("Arquivo de Log Incorreto - Fitness Diferentes!\n\n");
					exit(1);
				}

				popInicial->push_back(p);
			}
		}
		else
		{
			short int *flags = (short int*)malloc(nitens * sizeof(short int));
			int pos = 0, max = 0;

			for(int s = 0; s < npop; s++)
			{
				prob = (short int*)malloc(nitens * sizeof(short int));

				if(!fscanf (f, "%d\n", &valorTotal))
				{
					printf("Arquivo de Log Incorreto - Fitness!\n\n");
					exit(1);
				}

				for(int i = 0; i < nitens; i++)
				{
					if(!fscanf (f, "%hd", &flags[i]))
					{
						printf("Arquivo de Log Incorreto - Flag!\n\n");
						exit(1);
					}
				}

				for(int i = 0; i < nitens; i++)
				{
					if(flags[i] == 1)
						prob[pos++] = i;
				}

				max = pos;

				for(int i = 0; i < nitens; i++)
				{
					if(flags[i] == 0)
						prob[pos++] = i;
				}

				pos = 0;

				p = new KnapSack_1(prob, max);

				if(valorTotal != p->getFitness())
				{
					printf("Arquivo de Log Incorreto - Fitness Diferentes!\n\n");
					exit(1);
				}

				popInicial->push_back(p);
			}
			free(flags);
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

	fprintf(f, "%d %d %d %s\n\n", sizePop, KnapSack_1::nitens, KnapSack_1::ncontraint, "src_1");

	for(iter = popInicial->begin(); iter != popInicial->end(); iter++)
	{
		prob = dynamic_cast<KnapSack_1 *>(*iter)->getSoluction().ordemItens;

		fprintf(f, "%d\n", (int)dynamic_cast<KnapSack_1 *>(*iter)->getSoluction().fitness);

		fprintf(f, "%d ", (int)dynamic_cast<KnapSack_1 *>(*iter)->getSoluction().max);
		for(int i = 0; i < KnapSack_1::nitens; i++)
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
	KnapSack_1::values = (double*)malloc(KnapSack_1::nitens * sizeof(double));

	KnapSack_1::limit = (double*)malloc(KnapSack_1::ncontraint * sizeof(double));

	KnapSack_1::constraint = (double**)malloc(KnapSack_1::ncontraint * sizeof(double*));
	for(int i = 0; i < KnapSack_1::ncontraint; i++)
		KnapSack_1::constraint[i] = (double*)malloc(KnapSack_1::nitens * sizeof(double));
}

void Problema::desalocaMemoria()
{
	free(KnapSack_1::values);

	free(KnapSack_1::limit);

	for(int i = 0; i < KnapSack_1::ncontraint; i++)
		free(KnapSack_1::constraint[i]);
	free(KnapSack_1::constraint);
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

KnapSack_1::KnapSack_1() : Problema::Problema()
{
	sol.ordemItens = (short int*)malloc(nitens * sizeof(short int));

	for(register int i = 0; i < nitens; i++)
		sol.ordemItens[i] = i;

	random_shuffle(&sol.ordemItens[0], &sol.ordemItens[nitens]);

	sol.max = -1;

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}


KnapSack_1::KnapSack_1(short int *prob) : Problema::Problema()
{
	sol.ordemItens = prob;
	sol.max = -1;

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

KnapSack_1::KnapSack_1(short int *prob, int max) : Problema::Problema()
{
	sol.ordemItens = prob;

	sol.max = max;
	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

KnapSack_1::KnapSack_1(const Problema &prob) : Problema::Problema()
{
	KnapSack_1 *other = dynamic_cast<KnapSack_1 *>(const_cast<Problema *>(&prob));

	this->sol.ordemItens = (short int*)malloc(nitens * sizeof(short int));
	for(int i = 0; i < nitens; i++)
		this->sol.ordemItens[i] = other->sol.ordemItens[i];

	this->sol.max = other->sol.max;

	this->sol.fitness = other->sol.fitness;
	exec = prob.exec;
}

KnapSack_1::KnapSack_1(const Problema &prob, int pos1, int pos2) : Problema::Problema()
{
	KnapSack_1 *other = dynamic_cast<KnapSack_1 *>(const_cast<Problema *>(&prob));

	this->sol.ordemItens = (short int*)malloc(nitens * sizeof(short int));
	for(int i = 0; i < nitens; i++)
		this->sol.ordemItens[i] = other->sol.ordemItens[i];

	short int aux = this->sol.ordemItens[pos1];
	this->sol.ordemItens[pos1] = this->sol.ordemItens[pos2];
	this->sol.ordemItens[pos2] = aux;

	sol.max = -1;

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

KnapSack_1::~KnapSack_1()
{
	if(this->sol.ordemItens != NULL)
		free(this->sol.ordemItens);
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
inline bool KnapSack_1::calcFitness(bool esc)
{
	sol.fitness = 0;

	if(sol.max == -1)
	{
		double *tempConstraints = (double*)malloc(KnapSack_1::ncontraint * sizeof(double)), fitness = 0;
		for(register int i = 0; i < KnapSack_1::ncontraint; i++)
			tempConstraints[i] = 0;

		register int max = 0, item = 0;
		for(max = 0; max < nitens; max++)
		{
			item = sol.ordemItens[max];

			if(!constraintVerify(item, tempConstraints))
				break;

			fitness += KnapSack_1::values[item];
		}

		sort(&sol.ordemItens[0], &sol.ordemItens[max]);
		sort(&sol.ordemItens[max], &sol.ordemItens[nitens]);

		sol.fitness = fitness;
		sol.max = max;

		while(constraintVerify(sol.ordemItens[max], tempConstraints))
			fitness += KnapSack_1::values[sol.ordemItens[max++]];

		if(fitness != sol.fitness || max != sol.max)
		{
			sort(&sol.ordemItens[0], &sol.ordemItens[max]);
			sort(&sol.ordemItens[max], &sol.ordemItens[nitens]);

			sol.fitness = fitness;
			sol.max = max;
		}

		free(tempConstraints);
	}
	else
	{
		for(register int i = 0; i < sol.max; i++)
		{
			sol.fitness += KnapSack_1::values[sol.ordemItens[i]];
		}
	}

	return true;
}

bool KnapSack_1::operator == (const Problema& p)
{
	return this->getFitness() == p.getFitness();
}

bool KnapSack_1::operator != (const Problema& p)
{
	return this->getFitness() != p.getFitness();
}

bool KnapSack_1::operator <= (const Problema& p)
{
	return this->getFitness() <= p.getFitness();
}

bool KnapSack_1::operator >= (const Problema& p)
{
	return this->getFitness() >= p.getFitness();
}

bool KnapSack_1::operator < (const Problema& p)
{
	return this->getFitness() < p.getFitness();
}

bool KnapSack_1::operator > (const Problema& p)
{
	return this->getFitness() > p.getFitness();
}

inline void KnapSack_1::imprimir(bool esc)
{
	if(esc == true)
	{
		printf("sack: ");
		for(int i = 0; i < sol.max; i++)
			printf("|%d|", sol.ordemItens[i]);

		printf(" ==> %0.2f\n", sol.fitness);
	}
	else
	{
		for(int i = 0; i < nitens; i++)
			printf("%d ", sol.ordemItens[i]);
	}
	printf("\n");
}

/* Retorna um vizinho aleatorio da solucao atual. */
inline Problema* KnapSack_1::vizinho()
{
	int p1 = xRand(rand(), 0, sol.max), p2 = xRand(rand(), sol.max, nitens);
	Problema *prob = NULL;

	prob = new KnapSack_1(*this, p1, p2);

	return prob;
}

/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual. */
inline vector<pair<Problema*, InfoTabu*>* >* KnapSack_1::buscaLocal()
{
	if(KnapSack_1::num_vizinhos > MAX_PERMUTACOES)
		return buscaLocal((float)MAX_PERMUTACOES/(float)KnapSack_1::num_vizinhos);

	register int p1, p2;
	Problema *prob = NULL;
	pair<Problema*, InfoTabu*>* temp;
	vector<pair<Problema*, InfoTabu*>* >* local = new vector<pair<Problema*, InfoTabu*>* >();

	for(p1 = 0; p1 < sol.max; p1++)
	{
		for(p2 = sol.max; p2 < nitens; p2++)
		{
			prob = new KnapSack_1(*this, p1, p2);

			temp = new pair<Problema*, InfoTabu*>();
			temp->first = prob;
			temp->second = new InfoTabu_KnapSack_1(p1, p2);

			local->push_back(temp);
		}
	}

	random_shuffle(local->begin(), local->end());
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Retorna um conjunto de com uma parcela das solucoes viaveis vizinhas da atual. */
inline vector<pair<Problema*, InfoTabu*>* >* KnapSack_1::buscaLocal(float parcela)
{
	Problema *prob = NULL;
	int p1, p2;
	int numItens = nitens;
	pair<Problema*, InfoTabu*>* temp;
	vector<pair<Problema*, InfoTabu*>* >* local = new vector<pair<Problema*, InfoTabu*>* >();
	int def;

	def = (int)((float)KnapSack_1::num_vizinhos*parcela);

	if(def > MAX_PERMUTACOES)
		def = MAX_PERMUTACOES;

	for(register int i = 0; i < def; i++)
	{
		p1 = xRand(rand(), 0, sol.max), p2 = xRand(rand(), sol.max, numItens);

		prob = new KnapSack_1(*this, p1, p2);

		temp = new pair<Problema*, InfoTabu*>();
		temp->first = prob;
		temp->second = new InfoTabu_KnapSack_1(p1, p2);

		local->push_back(temp);
	}
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Realiza um crossover com uma outra solucao. Usa 2 pivos. */
inline pair<Problema*, Problema*>* KnapSack_1::crossOver(const Problema* parceiro, int tamParticao, int strength)
{
	short int *f1 = (short int*)malloc(nitens * sizeof(short int)), *f2 = (short int*)malloc(nitens * sizeof(short int));
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = tamParticao == 0 ? (nitens)/2 : tamParticao;
	int inicioPart = 0, fimPart = 0;

	KnapSack_1 *other = dynamic_cast<KnapSack_1 *>(const_cast<Problema *>(parceiro));

	inicioPart = xRand(rand(), 0, nitens);
	fimPart = inicioPart+particao <= nitens ? inicioPart+particao : nitens;

	swap_vect(this->sol.ordemItens, other->sol.ordemItens, f1, inicioPart, fimPart-inicioPart);
	swap_vect(other->sol.ordemItens, this->sol.ordemItens, f2, inicioPart, fimPart-inicioPart);

	filhos->first = new KnapSack_1(f1);
	filhos->second = new KnapSack_1(f2);

	return filhos;
}

/* Realiza um crossover com uma outra solucao. Usa 1 pivo. */
inline pair<Problema*, Problema*>* KnapSack_1::crossOver(const Problema* parceiro, int strength)
{
	short int *f1 = (short int*)malloc(nitens * sizeof(short int)), *f2 = (short int*)malloc(nitens * sizeof(short int));
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = 0;

	KnapSack_1 *other = dynamic_cast<KnapSack_1 *>(const_cast<Problema *>(parceiro));

	particao = xRand(rand(), 1, nitens);

	swap_vect(this->sol.ordemItens, other->sol.ordemItens, f1, 0, particao);
	swap_vect(other->sol.ordemItens, this->sol.ordemItens, f2, 0, particao);

	filhos->first = new KnapSack_1(f1);
	filhos->second = new KnapSack_1(f2);

	return filhos;
}

/* Devolve uma mutacao aleatoria na solucao atual. */
inline Problema* KnapSack_1::mutacao(int mutMax)
{
	short int *mut = (short int*)malloc(nitens * sizeof(short int));
	Problema* vizinho = NULL, *temp = NULL, *mutacao = NULL;

	for(int i = 0; i < nitens; i++)
		mut[i] = sol.ordemItens[i];

	temp = new KnapSack_1(mut);
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

inline double KnapSack_1::getFitness() const
{
	return sol.fitness;
}

inline double KnapSack_1::getFitnessMaximize() const
{
	return (double)sol.fitness;
}

inline double KnapSack_1::getFitnessMinimize() const
{
	return (double)INV_FITNESS/sol.fitness;
}

/* Auxiliares */

inline void swap_vect(short int* p1, short int* p2, short int* f, int pos, int tam)
{
	for(register int i = pos; i < pos+tam; i++)
		f[i] = p1[i];

	for(register int i = 0, j = 0; i < KnapSack_1::nitens && j < KnapSack_1::nitens; i++)
	{
		if(j == pos)
			j = pos+tam;

		if(find(&p1[pos], &p1[pos+tam], p2[i]) == &p1[pos+tam])
			f[j++] = p2[i];
	}
	return;
}


inline bool constraintVerify(int item, double *constraints)
{
	register int c = 0;
	bool valido = true;

	for(c = 0; c < KnapSack_1::ncontraint; c++)
	{
		constraints[c] += KnapSack_1::constraint[c][item];

		if(constraints[c] > KnapSack_1::limit[c])
		{
			valido = false;
			break;
		}
	}

	if(valido == false)
		for(register int i = 0; i <= c; i++)
			constraints[i] -= KnapSack_1::constraint[i][item];

	return valido;
}

// comparator function:
bool fnequal1(Problema* prob1, Problema* prob2)
{
	KnapSack_1 *p1 = dynamic_cast<KnapSack_1 *>(const_cast<Problema *>(prob1));
	KnapSack_1 *p2 = dynamic_cast<KnapSack_1 *>(const_cast<Problema *>(prob2));

	if(p1->sol.fitness == p2->sol.fitness)
	{
		for(int i = 0; i < KnapSack_1::nitens; i++)
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
	KnapSack_1 *p1 = dynamic_cast<KnapSack_1 *>(const_cast<Problema *>(prob1));
	KnapSack_1 *p2 = dynamic_cast<KnapSack_1 *>(const_cast<Problema *>(prob2));

	return p1->sol.fitness == p2->sol.fitness;
}

// comparator function:
bool fncomp1(Problema *prob1, Problema *prob2)
{
	KnapSack_1 *p1 = dynamic_cast<KnapSack_1 *>(const_cast<Problema *>(prob1));
	KnapSack_1 *p2 = dynamic_cast<KnapSack_1 *>(const_cast<Problema *>(prob2));

	if(p1->sol.fitness == p2->sol.fitness)
	{
		for(int i = 0; i < KnapSack_1::nitens; i++)
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
	KnapSack_1 *p1 = dynamic_cast<KnapSack_1 *>(const_cast<Problema *>(prob1));
	KnapSack_1 *p2 = dynamic_cast<KnapSack_1 *>(const_cast<Problema *>(prob2));

	return p1->sol.fitness > p2->sol.fitness;
}


inline bool ptcomp(pair<Problema*, InfoTabu*>* p1, pair<Problema*, InfoTabu*>* p2)
{
	return (p1->first->getFitness() < p2->first->getFitness());
}
