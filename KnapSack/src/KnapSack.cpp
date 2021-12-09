#include "KnapSack.hpp"

using namespace std;

/* Static Members */

ProblemType Problem::TIPO = MAXIMIZACAO;

double Problem::best = 0;
double Problem::worst = 0;
int Problem::numInst = 0;
long int Problem::totalNumInst = 0;

char KnapSack::name[128];
double *KnapSack::values = NULL, **KnapSack::constraint = NULL, *KnapSack::limit = NULL;
int KnapSack::nitens = 0, KnapSack::ncontraint = 0;

int KnapSack::num_vizinhos = 0;

Problem* Problem::randSoluction()
{
	return new KnapSack();
}

Problem* Problem::copySoluction(const Problem& prob)
{
	return new KnapSack(prob);
}


void Problem::readProblemFromFile(char* input)
{
	FILE *f = fopen(input, "r");

	if(f == NULL)
		throw "Wrong Data File!";

	if(!fgets (KnapSack::name, 128, f))
		throw "Wrong Data File!";

	if(!fscanf (f, "%d %d %*d\n", &KnapSack::nitens, &KnapSack::ncontraint))
		throw "Wrong Data File!";

	Problem::alocaMemoria();

	for(int i = 0; i < KnapSack::nitens; i++)
	{
		if (!fscanf (f, "%lf", &KnapSack::values[i]))
			throw "Wrong Data File!";
	}

	double **constraints = (double**)malloc(KnapSack::ncontraint * sizeof(double*));
	for(int i = 0; i < KnapSack::ncontraint; i++)
		constraints[i] = (double*)malloc(KnapSack::nitens * sizeof(double));

	for(int i = 0; i < KnapSack::ncontraint; i++)
	{
		for (int j = 0; j < KnapSack::nitens; j++)
		{
			if (!fscanf (f, "%lf", &constraints[i][j]))
				throw "Wrong Data File!";
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
			throw "Wrong Data File!";
	}

	for(int i = 1; i < KnapSack::nitens + 2; i++)
		KnapSack::num_vizinhos += i;

	return;
}

list<Problem*>* Problem::readPopulationFromLog(char *log)
{
	FILE *f = fopen(log, "r");

	if(f != NULL)
	{
		list<Problem*>* popInicial = new list<Problem*>();
		int npop, nitens, nconstraint, valorTotal, limit;
		char format_type[32];
		short int *prob;
		Problem* p;

		if(!fscanf (f, "%d %d %d %s\n", &npop, &nitens, &nconstraint, format_type))
			throw "Wrong Log File!";

		if(nitens != KnapSack::nitens || nconstraint != KnapSack::ncontraint)
			throw "Wrong Log File!";

		for(int s = 0; s < npop; s++)
		{
			prob = (short int*)malloc(nitens * sizeof(short int));

			if(!fscanf (f, "%d\n", &valorTotal))
				throw "Wrong Log File!";

			if(!fscanf (f, "%d ", &limit))
				throw "Wrong Log File!";

			for(int i = 0; i < nitens; i++)
			{
				if(!fscanf (f, "%hd ", &prob[i]))
					throw "Wrong Log File!";
			}

			p = new KnapSack(prob, limit);
			if(valorTotal != p->getFitness())
				throw "Wrong Log File!";

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

void Problem::dumpCurrentPopulationInLog(char *log, list<Problem*>* popInicial)
{
	FILE *f = fopen(log, "w");

	if(f != NULL)
	{
		int sizePop = (int)popInicial->size();
		list<Problem*>::iterator iter;
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
}

void Problem::writeResultInFile(char *dados, char *parametros, ExecInfo *info, char *resultado)
{
	FILE *f;


	if(*resultado != '\0')
	{
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
}

void Problem::alocaMemoria()
{
	KnapSack::values = (double*)malloc(KnapSack::nitens * sizeof(double));

	KnapSack::limit = (double*)malloc(KnapSack::ncontraint * sizeof(double));

	KnapSack::constraint = (double**)malloc(KnapSack::nitens * sizeof(double*));
	for(int i = 0; i < KnapSack::nitens; i++)
		KnapSack::constraint[i] = (double*)malloc(KnapSack::ncontraint * sizeof(double));
}

void Problem::unloadMemory()
{
	free(KnapSack::values);

	free(KnapSack::limit);

	for(int i = 0; i < KnapSack::ncontraint; i++)
		free(KnapSack::constraint[i]);
	free(KnapSack::constraint);
}


/* Metodos */

KnapSack::KnapSack() : Problem::Problem()
{
	sol.ordemItens = (short int*)malloc(nitens * sizeof(short int));

	for(int i = 0; i < nitens; i++)
		sol.ordemItens[i] = i;

	random_shuffle(&sol.ordemItens[0], &sol.ordemItens[nitens]);

	sol.limit = -1;

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

KnapSack::KnapSack(short int *prob) : Problem::Problem()
{
	sol.ordemItens = prob;
	sol.limit = -1;

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

KnapSack::KnapSack(short int *prob, int limit) : Problem::Problem()
{
	sol.ordemItens = prob;
	sol.limit = limit;

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

KnapSack::KnapSack(const Problem &prob) : Problem::Problem()
{
	KnapSack *other = dynamic_cast<KnapSack *>(const_cast<Problem *>(&prob));

	this->sol.ordemItens = (short int*)malloc(nitens * sizeof(short int));
	for(int i = 0; i < nitens; i++)
		this->sol.ordemItens[i] = other->sol.ordemItens[i];

	this->sol.limit = other->sol.limit;

	this->sol.fitness = other->sol.fitness;
	exec = prob.exec;
}

KnapSack::KnapSack(const Problem &prob, int pos1, int pos2) : Problem::Problem()
{
	KnapSack *other = dynamic_cast<KnapSack *>(const_cast<Problem *>(&prob));

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

		double fitness = 0;
		int item = 0, limit = 0;
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
		for(int i = 0; i < sol.limit; i++)
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
inline Problem* KnapSack::vizinho()
{
	int p1 = xRand(0, sol.limit), p2 = xRand(sol.limit, nitens);
	Problem *prob = NULL;

	prob = new KnapSack(*this, p1, p2);

	return prob;
}

/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual. */
inline vector<pair<Problem*, InfoTabu*>* >* KnapSack::buscaLocal()
{
	if(KnapSack::num_vizinhos > MAX_PERMUTACOES)
		return buscaLocal((float)MAX_PERMUTACOES/(float)KnapSack::num_vizinhos);

	int p1, p2;
	Problem *prob = NULL;
	pair<Problem*, InfoTabu*>* temp;
	vector<pair<Problem*, InfoTabu*>* >* local = new vector<pair<Problem*, InfoTabu*>* >();

	for(p1 = 0; p1 < sol.limit; p1++)
	{
		for(p2 = sol.limit; p2 < nitens; p2++)
		{
			prob = new KnapSack(*this, p1, p2);

			temp = new pair<Problem*, InfoTabu*>();
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
inline vector<pair<Problem*, InfoTabu*>* >* KnapSack::buscaLocal(float parcela)
{
	Problem *prob = NULL;
	int p1, p2;
	int numItens = nitens;
	pair<Problem*, InfoTabu*>* temp;
	vector<pair<Problem*, InfoTabu*>* >* local = new vector<pair<Problem*, InfoTabu*>* >();
	int def;

	def = (int)((float)KnapSack::num_vizinhos*parcela);

	if(def > MAX_PERMUTACOES)
		def = MAX_PERMUTACOES;

	for(int i = 0; i < def; i++)
	{
		p1 = xRand(0, sol.limit), p2 = xRand(sol.limit, numItens);

		prob = new KnapSack(*this, p1, p2);

		temp = new pair<Problem*, InfoTabu*>();
		temp->first = prob;
		temp->second = new InfoTabu_KnapSack(p1, p2);

		local->push_back(temp);
	}
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Realiza um crossover com uma outra solucao. Usa 2 pivos. */
inline pair<Problem*, Problem*>* KnapSack::crossOver(const Problem* parceiro, int tamParticao, int strength)
{
	short int *f1 = (short int*)malloc(nitens * sizeof(short int)), *f2 = (short int*)malloc(nitens * sizeof(short int));
	pair<Problem*, Problem*>* filhos = new pair<Problem*, Problem*>();
	int particao = tamParticao == 0 ? (nitens)/2 : tamParticao;
	int inicioPart = 0, fimPart = 0;

	KnapSack *other = dynamic_cast<KnapSack *>(const_cast<Problem *>(parceiro));

	inicioPart = xRand(0, nitens);
	fimPart = inicioPart+particao <= nitens ? inicioPart+particao : nitens;

	swap_vect(this->sol.ordemItens, other->sol.ordemItens, f1, inicioPart, fimPart-inicioPart);
	swap_vect(other->sol.ordemItens, this->sol.ordemItens, f2, inicioPart, fimPart-inicioPart);

	filhos->first = new KnapSack(f1);
	filhos->second = new KnapSack(f2);

	return filhos;
}

/* Realiza um crossover com uma outra solucao. Usa 1 pivo. */
inline pair<Problem*, Problem*>* KnapSack::crossOver(const Problem* parceiro, int strength)
{
	short int *f1 = (short int*)malloc(nitens * sizeof(short int)), *f2 = (short int*)malloc(nitens * sizeof(short int));
	pair<Problem*, Problem*>* filhos = new pair<Problem*, Problem*>();
	int particao = 0;

	KnapSack *other = dynamic_cast<KnapSack *>(const_cast<Problem *>(parceiro));

	particao = xRand(1, nitens);

	swap_vect(this->sol.ordemItens, other->sol.ordemItens, f1, 0, particao);
	swap_vect(other->sol.ordemItens, this->sol.ordemItens, f2, 0, particao);

	filhos->first = new KnapSack(f1);
	filhos->second = new KnapSack(f2);

	return filhos;
}

/* Devolve uma mutacao aleatoria na solucao atual. */
inline Problem* KnapSack::mutacao(int mutMax)
{
	short int *mut = (short int*)malloc(nitens * sizeof(short int));
	Problem* vizinho = NULL, *temp = NULL, *mutacao = NULL;

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
	for(int i = pos; i < pos+tam; i++)
		f[i] = p1[i];

	for(int i = 0, j = 0; i < KnapSack::nitens && j < KnapSack::nitens; i++)
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
	int c = 0;

	for(constraint = constraints.begin(); constraint != constraints.end(); constraint++)
	{
		*constraint += KnapSack::constraint[item][c];

		if(*constraint > KnapSack::limit[c++])
			return false;
	}

	return true;
}

// comparator function:
bool fnequal1(Problem* prob1, Problem* prob2)
{
	KnapSack *p1 = dynamic_cast<KnapSack *>(const_cast<Problem *>(prob1));
	KnapSack *p2 = dynamic_cast<KnapSack *>(const_cast<Problem *>(prob2));

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
bool fnequal2(Problem* prob1, Problem* prob2)
{
	KnapSack *p1 = dynamic_cast<KnapSack *>(const_cast<Problem *>(prob1));
	KnapSack *p2 = dynamic_cast<KnapSack *>(const_cast<Problem *>(prob2));

	return p1->sol.fitness == p2->sol.fitness;
}

// comparator function:
bool fncomp1(Problem *prob1, Problem *prob2)
{
	KnapSack *p1 = dynamic_cast<KnapSack *>(const_cast<Problem *>(prob1));
	KnapSack *p2 = dynamic_cast<KnapSack *>(const_cast<Problem *>(prob2));

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
bool fncomp2(Problem *prob1, Problem *prob2)
{
	KnapSack *p1 = dynamic_cast<KnapSack *>(const_cast<Problem *>(prob1));
	KnapSack *p2 = dynamic_cast<KnapSack *>(const_cast<Problem *>(prob2));

	return p1->sol.fitness > p2->sol.fitness;
}

inline bool ptcomp(pair<Problem*, InfoTabu*>* p1, pair<Problem*, InfoTabu*>* p2)
{
	return (p1->first->getFitness() < p2->first->getFitness());
}
