#include "TravellingSalesman.hpp"

using namespace std;

/* Static Members */

ProblemType Problema::TIPO = MINIMIZACAO;

double Problema::best = 0;
double Problema::worst = 0;
int Problema::numInst = 0;
long int Problema::totalNumInst = 0;

char TravellingSalesman::name[128];
double **TravellingSalesman::edges = NULL;
int TravellingSalesman::nnodes = 0;

int TravellingSalesman::num_vizinhos = 0;

Problema* Problema::randSoluction()
{
	return new TravellingSalesman();
}

Problema* Problema::copySoluction(const Problema& prob)
{
	return new TravellingSalesman(prob);
}


void Problema::leProblema(char* input)
{
	FILE *f = fopen(input, "r");

	if(f == NULL)
		exit(1);

	char edge_weight_format[32];
	char *line = NULL;
	size_t size = 0;

	while((getline(&line, &size, f)) != -1)
	{
		if(strstr(line, "NAME: "))
			sscanf(line + strlen("NAME: "), "%s", TravellingSalesman::name);

		if(strstr(line, "DIMENSION: "))
			sscanf(line + strlen("DIMENSION: "), "%d", &TravellingSalesman::nnodes);

		if(strstr(line, "EDGE_WEIGHT_TYPE: "))
			sscanf(line + strlen("EDGE_WEIGHT_TYPE: "), "%s", edge_weight_format);

		if(strstr(line, "EDGE_WEIGHT_FORMAT: "))
			sscanf(line + strlen("EDGE_WEIGHT_FORMAT: "), "%s", edge_weight_format);

		if(strstr(line, "EDGE_WEIGHT_SECTION") || strstr(line, "NODE_COORD_SECTION"))
			break;
	}

	Problema::alocaMemoria();

	if(!strcmp(edge_weight_format, "FULL_MATRIX"))
	{
		for(int i = 0; i < TravellingSalesman::nnodes; i++)
		{
			for(int j = 0; j < TravellingSalesman::nnodes; j++)
			{
				if(!fscanf (f, "%lf", &TravellingSalesman::edges[i][j]))
					exit(1);

				if(i == j)
					TravellingSalesman::edges[i][j] = -1;
			}
		}
	}

	if(!strcmp(edge_weight_format, "UPPER_ROW"))
	{
		for(int i = 0; i < TravellingSalesman::nnodes; i++)
		{
			TravellingSalesman::edges[i][i] = -1;

			for(int j = i+1; j < TravellingSalesman::nnodes; j++)
			{
				if(!fscanf (f, "%lf", &TravellingSalesman::edges[i][j]))
					exit(1);

				TravellingSalesman::edges[j][i] = TravellingSalesman::edges[i][j];
			}
		}
	}

	if(!strcmp(edge_weight_format, "UPPER_DIAG_ROW"))
	{
		for(int i = 0; i < TravellingSalesman::nnodes; i++)
		{
			for(int j = i; j < TravellingSalesman::nnodes; j++)
			{
				if(!fscanf (f, "%lf", &TravellingSalesman::edges[i][j]))
					exit(1);

				TravellingSalesman::edges[j][i] = TravellingSalesman::edges[i][j];

				if(i == j)
					TravellingSalesman::edges[i][j] = -1;
			}
		}
	}

	if(!strcmp(edge_weight_format, "LOWER_ROW"))
	{
		for(int i = 0; i < TravellingSalesman::nnodes; i++)
		{
			TravellingSalesman::edges[i][i] = -1;

			for(int j = 0; j < i; j++)
			{
				if(!fscanf (f, "%lf", &TravellingSalesman::edges[i][j]))
					exit(1);

				TravellingSalesman::edges[j][i] = TravellingSalesman::edges[i][j];
			}
		}
	}

	if(!strcmp(edge_weight_format, "LOWER_DIAG_ROW"))
	{
		for(int i = 0; i < TravellingSalesman::nnodes; i++)
		{
			for(int j = 0; j <= i; j++)
			{
				if(!fscanf (f, "%lf", &TravellingSalesman::edges[i][j]))
					exit(1);

				TravellingSalesman::edges[j][i] = TravellingSalesman::edges[i][j];

				if(i == j)
					TravellingSalesman::edges[i][j] = -1;
			}
		}
	}

	if(!strcmp(edge_weight_format, "EUC_2D"))
	{
		double *X = (double*)malloc(TravellingSalesman::nnodes * sizeof(double));
		double *Y = (double*)malloc(TravellingSalesman::nnodes * sizeof(double));
		int no = 0;

		for(int i = 0; i < TravellingSalesman::nnodes; i++)
		{
			if(!fscanf (f, "%d %lf %lf", &no, &X[i], &Y[i]))
				exit(1);

			if(i+1 != no)
				exit(1);
		}

		double xd, yd;

		for(int i = 0; i < TravellingSalesman::nnodes; i++)
		{
			for(int j = 0; j < TravellingSalesman::nnodes; j++)
			{
				xd = X[i] - X[j];
				yd = Y[i] - Y[j];

				TravellingSalesman::edges[i][j] = sqrt(pow(xd, 2) + pow(yd, 2));
			}
		}

		free(X);
		free(Y);
	}

	for(int i = 1; i <= TravellingSalesman::nnodes; i++)
		TravellingSalesman::num_vizinhos += i;

	return;
}

list<Problema*>* Problema::lePopulacao(char *log)
{
	FILE *f = fopen(log, "r");

	if(f != NULL)
	{
		list<Problema*>* popInicial = new list<Problema*>();
		double peso;
		int npop, nnodes;
		short int *ordem;
		Problema* p;

		if(!fscanf (f, "%d %d", &npop, &nnodes))
		{
			printf("Wrong Log File!\n\n");
			exit(1);
		}

		if(nnodes != TravellingSalesman::nnodes)
		{
			printf("Wrong Log File!\n\n");
			exit(1);
		}

		for(int i = 0; i < npop; i++)
		{
			ordem = (short int*)malloc((nnodes+1) * sizeof(short int));

			if(!fscanf (f, "%lf", &peso))
			{
				printf("Wrong Log File!\n\n");
				exit(1);
			}

			for(int j = 0; j <= nnodes; j++)
			{
				if(!fscanf (f, "%hd", &ordem[j]))
				{
					printf("Wrong Log File!\n\n");
					exit(1);
				}
			}

			p = new TravellingSalesman(ordem);

			if(peso != p->getFitness())
			{
				printf("Wrong Log File!\n\n");
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
	short int *ordem;

	fprintf(f, "%d %d\n\n", sizePop, TravellingSalesman::nnodes);

	for(iter = popInicial->begin(); iter != popInicial->end(); iter++)
	{
		ordem = dynamic_cast<TravellingSalesman *>(*iter)->getSoluction().ordemNodes;

		fprintf(f, "%d\n", (int)dynamic_cast<TravellingSalesman *>(*iter)->getSoluction().fitness);

		for(int j = 0; j <= TravellingSalesman::nnodes; j++)
		{
			fprintf(f, "%hd ", ordem[j]);
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
	TravellingSalesman::edges = (double**)malloc(TravellingSalesman::nnodes * sizeof(double*));
	for(int i = 0; i < TravellingSalesman::nnodes; i++)
		TravellingSalesman::edges[i] = (double*)malloc(TravellingSalesman::nnodes * sizeof(double));
}

void Problema::desalocaMemoria()
{
	free(TravellingSalesman::edges);
}


/* Metodos */

TravellingSalesman::TravellingSalesman() : Problema::Problema()
{
	sol.ordemNodes = (short int*)malloc((nnodes+1) * sizeof(short int));
	sol.fitness = -1;

	if(xRand(0, 5) == 0)	// Tenta uma solucao gulosa
	{
		int noAtual = xRand(0, nnodes);
		int proxNo = 0;
		int pos = 0;
		double d = 0;

		sol.ordemNodes[pos++] = noAtual;
		for(int i = 0; i < nnodes; i++)
		{
			proxNo = -1;
			d = INT_MAX;

			for(int j = 0; j < nnodes && i != nnodes-1; j++)
			{
				if(edges[noAtual][j] != -1 && edges[noAtual][j] < d && find(&sol.ordemNodes[0], &sol.ordemNodes[pos], j) == &sol.ordemNodes[pos])
				{
					proxNo = j;
					d = edges[noAtual][proxNo];
				}
			}

			if(proxNo == -1)
			{
				if(i == nnodes-1)
					proxNo = sol.ordemNodes[0];
				else
					return;
			}

			sol.ordemNodes[pos++] = proxNo;
			noAtual = proxNo;
		}
	}
	else
	{
		for(int i = 0; i < nnodes; i++)
			sol.ordemNodes[i] = i;

		random_shuffle(&sol.ordemNodes[0], &sol.ordemNodes[nnodes]);
		sol.ordemNodes[nnodes] = sol.ordemNodes[0];
	}

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}


TravellingSalesman::TravellingSalesman(short int *prob) : Problema::Problema()
{
	sol.ordemNodes = prob;
	sol.fitness = -1;

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

TravellingSalesman::TravellingSalesman(const Problema &prob) : Problema::Problema()
{
	TravellingSalesman *other = dynamic_cast<TravellingSalesman *>(const_cast<Problema *>(&prob));

	this->sol.ordemNodes = (short int*)malloc((nnodes+1) * sizeof(short int));

	for(int i = 0; i <= nnodes; i++)
		this->sol.ordemNodes[i] = other->sol.ordemNodes[i];

	this->sol.fitness = other->sol.fitness;

	exec = prob.exec;
}

TravellingSalesman::TravellingSalesman(const Problema &prob, int pos1, int pos2) : Problema::Problema()
{
	TravellingSalesman *other = dynamic_cast<TravellingSalesman *>(const_cast<Problema *>(&prob));

	this->sol.ordemNodes = (short int*)malloc((nnodes+1) * sizeof(short int));

	for(int i = 0; i <= nnodes; i++)
		this->sol.ordemNodes[i] = other->sol.ordemNodes[i];

	short int aux = this->sol.ordemNodes[pos1];
	this->sol.ordemNodes[pos1] = this->sol.ordemNodes[pos2];
	this->sol.ordemNodes[pos2] = aux;

	if(pos1 == 0 || pos2 == 0)
		sol.ordemNodes[nnodes] = sol.ordemNodes[0];
	else if(pos1 == nnodes || pos2 == nnodes)
		sol.ordemNodes[0] = sol.ordemNodes[nnodes];

	sol.fitness = -1;
	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

TravellingSalesman::~TravellingSalesman()
{
	if(sol.ordemNodes != NULL)
		free(sol.ordemNodes);
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
inline bool TravellingSalesman::calcFitness(bool esc)
{
	double sumEdges = 0;

	double d;
	int c1, c2;
	for(int c = 0; c < nnodes; c++)
	{
		c1 = sol.ordemNodes[c];
		c2 = sol.ordemNodes[c+1];

		if((d = edges[c1][c2]) != -1)
			sumEdges += d;
		else
			return false;
	}

	sol.fitness = sumEdges;

	return true;
}

inline void TravellingSalesman::imprimir(bool esc)
{
	if(esc == false)
	{
		for(int j = 0; j <= nnodes; j++)
		{
			printf("%d ", sol.ordemNodes[j]);
		}

		printf("\n");
	}
	else
	{
		printf("|--> ");
		for(int j = 0; j < nnodes; j++)
		{
			printf("%d -> ", sol.ordemNodes[j]+1);
		}
		printf("%d |\n", sol.ordemNodes[nnodes]+1);
	}
}

/* Retorna um vizinho aleatorio da solucao atual. */
inline Problema* TravellingSalesman::vizinho()
{
	int p1 = xRand(0, nnodes+1), p2 = xRand(0, nnodes+1);
	Problema *prob = NULL;

	while(p2 == p1)
		p2 = xRand(0, nnodes+1);

	prob = new TravellingSalesman(*this, p1, p2);
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
inline vector<pair<Problema*, InfoTabu*>* >* TravellingSalesman::buscaLocal()
{
	if(TravellingSalesman::num_vizinhos > MAX_PERMUTACOES)
		return buscaLocal((float)MAX_PERMUTACOES/(float)TravellingSalesman::num_vizinhos);

	Problema *prob = NULL;
	int p1, p2;
	pair<Problema*, InfoTabu*>* temp;
	vector<pair<Problema*, InfoTabu*>* >* local = new vector<pair<Problema*, InfoTabu*>* >();

	for(p1 = 0; p1 < nnodes; p1++)
	{
		for(p2 = p1+1; p2 <= nnodes; p2++)
		{
			prob = new TravellingSalesman(*this, p1, p2);
			if(prob->getFitness() != -1)
			{
				temp = new pair<Problema*, InfoTabu*>();
				temp->first = prob;
				temp->second = new InfoTabu_TravellingSalesman(p1, p2);

				local->push_back(temp);
			}
			else
			{
				delete prob;
			}
		}
	}

	random_shuffle(local->begin(), local->end());
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Retorna um conjunto de com uma parcela das solucoes viaveis vizinhas da atual. */
inline vector<pair<Problema*, InfoTabu*>* >* TravellingSalesman::buscaLocal(float parcela)
{
	Problema *prob = NULL;
	int p1, p2;
	pair<Problema*, InfoTabu*>* temp;
	vector<pair<Problema*, InfoTabu*>* >* local = new vector<pair<Problema*, InfoTabu*>* >();
	int def;

	def = (int)((float)TravellingSalesman::num_vizinhos*parcela);

	if(def > MAX_PERMUTACOES)
		def = MAX_PERMUTACOES;

	for(int i = 0; i < def; i++)
	{
		p1 = xRand(0, nnodes+1), p2 = xRand(0, nnodes+1);

		while(p2 == p1)
			p2 = xRand(0, nnodes+1);

		prob = new TravellingSalesman(*this, p1, p2);
		if(prob->getFitness() != -1)
		{
			temp = new pair<Problema*, InfoTabu*>();
			temp->first = prob;
			temp->second = new InfoTabu_TravellingSalesman(p1, p2);

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
inline pair<Problema*, Problema*>* TravellingSalesman::crossOver(const Problema* parceiro, int tamParticao, int strength)
{
	short int *f1 = (short int*)malloc((nnodes+1) * sizeof(short int)), *f2 = (short int*)malloc((nnodes+1) * sizeof(short int));
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = tamParticao == 0 ? (TravellingSalesman::nnodes)/2 : tamParticao;
	int inicioPart = 0, fimPart = 0;

	TravellingSalesman *other = dynamic_cast<TravellingSalesman *>(const_cast<Problema *>(parceiro));

	inicioPart = xRand(0, nnodes);
	fimPart = inicioPart+particao <= nnodes ? inicioPart+particao : nnodes;

	swap_vect(this->sol.ordemNodes, other->sol.ordemNodes, f1, inicioPart, fimPart-inicioPart);
	swap_vect(other->sol.ordemNodes, this->sol.ordemNodes, f2, inicioPart, fimPart-inicioPart);

	filhos->first = new TravellingSalesman(f1);
	filhos->second = new TravellingSalesman(f2);

	return filhos;
}

/* Realiza um crossover com uma outra solucao. Usa 1 pivo. */
inline pair<Problema*, Problema*>* TravellingSalesman::crossOver(const Problema* parceiro, int strength)
{
	short int *f1 = (short int*)malloc((nnodes+1) * sizeof(short int)), *f2 = (short int*)malloc((nnodes+1) * sizeof(short int));
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = 0;

	TravellingSalesman *other = dynamic_cast<TravellingSalesman *>(const_cast<Problema *>(parceiro));

	particao = xRand(1, nnodes);

	swap_vect(this->sol.ordemNodes, other->sol.ordemNodes, f1, 0, particao);
	swap_vect(other->sol.ordemNodes, this->sol.ordemNodes, f2, 0, particao);

	filhos->first = new TravellingSalesman(f1);
	filhos->second = new TravellingSalesman(f2);

	return filhos;
}

/* Devolve uma mutacao aleatoria na solucao atual. */
inline Problema* TravellingSalesman::mutacao(int mutMax)
{
	short int *mut = (short int*)malloc((nnodes+1) * sizeof(short int));
	Problema* vizinho = NULL, *temp = NULL, *mutacao = NULL;

	for(int j = 0; j <= nnodes; j++)
		mut[j] = sol.ordemNodes[j];

	temp = new TravellingSalesman(mut);
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

inline double TravellingSalesman::getFitness() const
{
	return sol.fitness;
}

inline double TravellingSalesman::getFitnessMaximize() const
{
	return (double)INV_FITNESS/sol.fitness;
}

inline double TravellingSalesman::getFitnessMinimize() const
{
	return (double)sol.fitness;
}

/* Auxiliares */

inline void swap_vect(short int* p1, short int* p2, short int* f, int pos, int tam)
{
	for(int i = pos; i < pos+tam; i++)
		f[i] = p1[i];

	for(int i = 0, j = 0; i < TravellingSalesman::nnodes && j < TravellingSalesman::nnodes; i++)
	{
		if(j == pos)
			j = pos+tam;

		if(find(&p1[pos], &p1[pos+tam], p2[i]) == &p1[pos+tam])
			f[j++] = p2[i];
	}

	f[TravellingSalesman::nnodes] = f[0];

	return;
}

// comparator function:
bool fnequal1(Problema* prob1, Problema* prob2)
{
	TravellingSalesman *p1 = dynamic_cast<TravellingSalesman *>(prob1);
	TravellingSalesman *p2 = dynamic_cast<TravellingSalesman *>(prob2);

	if(p1->sol.fitness == p2->sol.fitness)
	{
		for(int j = 0; j <= TravellingSalesman::nnodes; j++)
			if(p1->sol.ordemNodes[j] != p2->sol.ordemNodes[j])
				return false;

		return true;
	}
	else
		return false;
}

// comparator function:
bool fnequal2(Problema* prob1, Problema* prob2)
{
	TravellingSalesman *p1 = dynamic_cast<TravellingSalesman *>(prob1);
	TravellingSalesman *p2 = dynamic_cast<TravellingSalesman *>(prob2);

	return p1->sol.fitness == p2->sol.fitness;
}

// comparator function:
bool fncomp1(Problema *prob1, Problema *prob2)
{
	TravellingSalesman *p1 = dynamic_cast<TravellingSalesman *>(prob1);
	TravellingSalesman *p2 = dynamic_cast<TravellingSalesman *>(prob2);

	if(p1->sol.fitness == p2->sol.fitness)
	{
		for(int j = 0; j <= TravellingSalesman::nnodes; j++)
			if(p1->sol.ordemNodes[j] != p2->sol.ordemNodes[j])
				return p1->sol.ordemNodes[j] < p2->sol.ordemNodes[j];

		return false;
	}
	else
		return p1->sol.fitness < p2->sol.fitness;
}

// comparator function:
bool fncomp2(Problema *prob1, Problema *prob2)
{
	TravellingSalesman *p1 = dynamic_cast<TravellingSalesman *>(prob1);
	TravellingSalesman *p2 = dynamic_cast<TravellingSalesman *>(prob2);

	return p1->sol.fitness < p2->sol.fitness;
}

inline bool ptcomp(pair<Problema*, InfoTabu*>* p1, pair<Problema*, InfoTabu*>* p2)
{
	return (p1->first->getFitness() > p2->first->getFitness());
}
