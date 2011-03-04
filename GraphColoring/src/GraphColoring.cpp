#include "GraphColoring.h"

using namespace std;

/* Static Members */

ProblemType Problema::TIPO = MINIMIZACAO;

double Problema::best = 0;
double Problema::worst = 0;
int Problema::numInst = 0;
long int Problema::totalNumInst = 0;

char GraphColoring::name[128];
vector<int> **GraphColoring::edges = NULL;
int GraphColoring::nedges = 0, GraphColoring::nnodes = 0;

int GraphColoring::num_vizinhos = 0;

Problema* Problema::randSoluction()
{
	return new GraphColoring();
}

Problema* Problema::copySoluction(const Problema& prob)
{
	return new GraphColoring(prob);
}


void Problema::leProblema(FILE *f)
{
	char tempLine[256];
	int n1, n2;

	while(fgets(tempLine, 128, f))
	{
		if(tempLine[0] == 'p')
		{
			sscanf(tempLine, "%*c %s %d %d\n", GraphColoring::name, &GraphColoring::nnodes, &GraphColoring::nedges);

			Problema::alocaMemoria();
		}

		if(tempLine[0] == 'e')
		{
			sscanf(tempLine, "%*c %d %d", &n1, &n2);

			GraphColoring::edges[n1]->push_back(n2);
			GraphColoring::edges[n2]->push_back(n1);
		}
	}

	for(int i = 1; i < GraphColoring::nnodes; i++)
		GraphColoring::num_vizinhos += i;

	return;
}

list<Problema*>* Problema::lePopulacao(char *log)
{
	FILE *f = fopen(log, "r");

	if(f != NULL)
	{
		list<Problema*>* popInicial = new list<Problema*>();
		int npop, nnodes, nedges, ncolors;
		short int *ordem;
		Problema* p;

		if(!fscanf (f, "%d %d %d", &npop, &nnodes, &nedges))
		{
			printf("Arquivo de log incorreto!\n\n");
			exit(1);
		}

		if(nnodes != GraphColoring::nnodes || nedges != GraphColoring::nedges)
		{
			printf("Arquivo de log incorreto!\n\n");
			exit(1);
		}

		for(int i = 0; i < npop; i++)
		{
			ordem = (short int*)malloc(nnodes * sizeof(short int));

			if(!fscanf (f, "%d", &ncolors))
			{
				printf("Arquivo de log incorreto!\n\n");
				exit(1);
			}

			for(int j = 0; j < nnodes; j++)
			{
				if(!fscanf (f, "%hd", &ordem[j]))
				{
					printf("Arquivo de log incorreto!\n\n");
					exit(1);
				}
			}

			p = new GraphColoring(ordem);

			if(ncolors != p->getFitness())
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
	short int *ordem;

	fprintf(f, "%d %d %d\n\n", sizePop, GraphColoring::nnodes, GraphColoring::nedges);

	for(iter = popInicial->begin(); iter != popInicial->end(); iter++)
	{
		ordem = dynamic_cast<GraphColoring *>(*iter)->getSoluction().ordemNodes;

		fprintf(f, "%d\n", (int)dynamic_cast<GraphColoring *>(*iter)->getSoluction().fitness);

		for(int j = 0; j < GraphColoring::nnodes; j++)
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
	GraphColoring::edges = (vector<int>**)malloc((1 + GraphColoring::nnodes) * sizeof(vector<int>*));

	for(int i = 1; i <= GraphColoring::nnodes; i++)
		GraphColoring::edges[i] = new vector<int>();
}

void Problema::desalocaMemoria()
{
	for(int i = 1; i <= GraphColoring::nnodes; i++)
			delete GraphColoring::edges[i];

	free(GraphColoring::edges);
}


/* Metodos */

GraphColoring::GraphColoring() : Problema::Problema()
{
	sol.ordemNodes = (short int*)malloc(nnodes * sizeof(short int));

	for(int i = 0; i < nnodes; i++)
		sol.ordemNodes[i] = i + 1;

	random_shuffle(&sol.ordemNodes[0], &sol.ordemNodes[nnodes]);

	sol.colors = NULL;
	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}


GraphColoring::GraphColoring(short int *prob) : Problema::Problema()
{
	sol.ordemNodes = prob;

	sol.colors = NULL;
	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

GraphColoring::GraphColoring(const Problema &prob) : Problema::Problema()
{
	GraphColoring *other = dynamic_cast<GraphColoring *>(const_cast<Problema *>(&prob));

	this->sol.ordemNodes = (short int*)malloc(nnodes * sizeof(short int));

	for(int i = 0; i < nnodes; i++)
		this->sol.ordemNodes[i] = other->sol.ordemNodes[i];

	this->sol.colors = NULL;
	this->sol.fitness = other->sol.fitness;

	if(other->sol.colors != NULL)
	{
		this->sol.colors = (short int*)malloc(nnodes * sizeof(short int));

		for(int i = 0; i < nnodes; i++)
			this->sol.colors[i] = other->sol.colors[i];
	}
	exec = prob.exec;
}

GraphColoring::GraphColoring(const Problema &prob, int pos1, int pos2) : Problema::Problema()
{
	GraphColoring *other = dynamic_cast<GraphColoring *>(const_cast<Problema *>(&prob));

	this->sol.ordemNodes = (short int*)malloc(nnodes * sizeof(short int));

	for(int i = 0; i < nnodes; i++)
		this->sol.ordemNodes[i] = other->sol.ordemNodes[i];

	short int aux = this->sol.ordemNodes[pos1];
	this->sol.ordemNodes[pos1] = this->sol.ordemNodes[pos2];
	this->sol.ordemNodes[pos2] = aux;

	this->sol.colors = NULL;
	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

GraphColoring::~GraphColoring()
{
	if(sol.ordemNodes != NULL)
		free(sol.ordemNodes);

	if(sol.colors != NULL)
		free(sol.colors);
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
inline bool GraphColoring::calcFitness(bool esc)
{
	short int *aux_colors = (short int*)malloc((nnodes + 1) * sizeof(short int));

	for(register int i = 1; i <= nnodes; i++)
		aux_colors[i] = 0;

	int minColor = 1;
	aux_colors[0] = 1;

	int noCorrente, corCorrente;
	vector<int>::iterator vizinhos;
	for(register int n = 0; n < nnodes; n++)
	{
		noCorrente = sol.ordemNodes[n];
		corCorrente = -1;

		for(register int c = 1; c <= minColor; c++)
		{
			for(vizinhos = edges[noCorrente]->begin(); vizinhos != edges[noCorrente]->end() && aux_colors[*vizinhos] != c; vizinhos++);

			if(vizinhos == edges[noCorrente]->end())
			{
				corCorrente = c;
				break;
			}
		}

		if(corCorrente < 0)
			corCorrente = ++minColor;

		aux_colors[noCorrente] = corCorrente;
	}

	aux_colors[0] = minColor;
	sol.fitness = minColor;

	if(esc == false)
		free(aux_colors);
	else
		sol.colors = aux_colors;

	return true;
}

inline void GraphColoring::imprimir(bool esc)
{
	if(esc == true)
	{
		calcFitness(esc);

		for(int i = 1; i <= sol.fitness; i++)
		{
			printf("color %.2d: ", i);
			for(int j = 1; j <= nnodes; j++)
			{
				if(sol.colors[j] == i)
					printf("|%d|", j);
			}
			printf("\n");
		}
	}
	else
	{
		for(int j = 0; j < nnodes; j++)
		{
			printf("%d ", sol.ordemNodes[j]);
		}
	}
	printf("\n");
}

/* Retorna um vizinho aleatorio da solucao atual. */
inline Problema* GraphColoring::vizinho()
{
	int p1 = xRand(rand(), 0, nnodes), p2 = xRand(rand(), 0, nnodes);
	Problema *prob = NULL;

	while(p2 == p1)
		p2 = xRand(rand(), 0, nnodes);

	prob = new GraphColoring(*this, p1, p2);

	return prob;
}

/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual. */
inline vector<pair<Problema*, InfoTabu*>* >* GraphColoring::buscaLocal()
{
	if(GraphColoring::num_vizinhos > MAX_PERMUTACOES)
		return buscaLocal((float)MAX_PERMUTACOES/(float)GraphColoring::num_vizinhos);

	Problema *prob = NULL;
	register int p1, p2;
	pair<Problema*, InfoTabu*>* temp;
	vector<pair<Problema*, InfoTabu*>* >* local = new vector<pair<Problema*, InfoTabu*>* >();

	for(p1 = 0; p1 < nnodes-1; p1++)
	{
		for(p2 = p1+1; p2 < nnodes; p2++)
		{
			prob = new GraphColoring(*this, p1, p2);

			temp = new pair<Problema*, InfoTabu*>();
			temp->first = prob;
			temp->second = new InfoTabu_GraphColoring(p1, p2);

			local->push_back(temp);
		}
	}

	random_shuffle(local->begin(), local->end());
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Retorna um conjunto de com uma parcela das solucoes viaveis vizinhas da atual. */
inline vector<pair<Problema*, InfoTabu*>* >* GraphColoring::buscaLocal(float parcela)
{
	Problema *prob = NULL;
	int p1, p2;
	pair<Problema*, InfoTabu*>* temp;
	vector<pair<Problema*, InfoTabu*>* >* local = new vector<pair<Problema*, InfoTabu*>* >();
	int def;

	def = (int)((float)GraphColoring::num_vizinhos*parcela);

	if(def > MAX_PERMUTACOES)
		def = MAX_PERMUTACOES;

	for(register int i = 0; i < def; i++)
	{
		p1 = xRand(rand(), 0, nnodes), p2 = xRand(rand(), 0, nnodes);

		while(p2 == p1)
			p2 = xRand(rand(), 0, nnodes);

		prob = new GraphColoring(*this, p1, p2);

		temp = new pair<Problema*, InfoTabu*>();
		temp->first = prob;
		temp->second = new InfoTabu_GraphColoring(p1, p2);

		local->push_back(temp);
	}
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Realiza um crossover com uma outra solucao. Usa 2 pivos. */
inline pair<Problema*, Problema*>* GraphColoring::crossOver(const Problema* parceiro, int tamParticao, int strength)
{
	short int *f1 = (short int*)malloc(nnodes * sizeof(short int)), *f2 = (short int*)malloc(nnodes * sizeof(short int));
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = tamParticao == 0 ? (GraphColoring::nnodes)/2 : tamParticao;
	int inicioPart = 0, fimPart = 0;

	GraphColoring *other = dynamic_cast<GraphColoring *>(const_cast<Problema *>(parceiro));

	inicioPart = xRand(rand(), 0, nnodes);
	fimPart = inicioPart+particao <= nnodes ? inicioPart+particao : nnodes;

	swap_vect(this->sol.ordemNodes, other->sol.ordemNodes, f1, inicioPart, fimPart-inicioPart);
	swap_vect(other->sol.ordemNodes, this->sol.ordemNodes, f2, inicioPart, fimPart-inicioPart);

	filhos->first = new GraphColoring(f1);
	filhos->second = new GraphColoring(f2);

	return filhos;
}

/* Realiza um crossover com uma outra solucao. Usa 1 pivo. */
inline pair<Problema*, Problema*>* GraphColoring::crossOver(const Problema* parceiro, int strength)
{
	short int *f1 = (short int*)malloc(nnodes * sizeof(short int)), *f2 = (short int*)malloc(nnodes * sizeof(short int));
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = 0;

	GraphColoring *other = dynamic_cast<GraphColoring *>(const_cast<Problema *>(parceiro));

	particao = xRand(rand(), 1, nnodes);

	swap_vect(this->sol.ordemNodes, other->sol.ordemNodes, f1, 0, particao);
	swap_vect(other->sol.ordemNodes, this->sol.ordemNodes, f2, 0, particao);

	filhos->first = new GraphColoring(f1);
	filhos->second = new GraphColoring(f2);

	return filhos;
}

/* Devolve uma mutacao aleatoria na solucao atual. */
inline Problema* GraphColoring::mutacao(int mutMax)
{
	short int *mut = (short int*)malloc(nnodes * sizeof(short int));
	Problema* vizinho = NULL, *temp = NULL, *mutacao = NULL;

	for(int j = 0; j < nnodes; j++)
		mut[j] = sol.ordemNodes[j];

	temp = new GraphColoring(mut);
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

inline double GraphColoring::getFitness() const
{
	return sol.fitness;
}

inline double GraphColoring::getFitnessMaximize() const
{
	return (double)INV_FITNESS/sol.fitness;
}

inline double GraphColoring::getFitnessMinimize() const
{
	return (double)sol.fitness;
}

/* Auxiliares */

inline void swap_vect(short int* p1, short int* p2, short int* f, int pos, int tam)
{
	for(register int i = pos; i < pos+tam; i++)
		f[i] = p1[i];

	for(register int i = 0, j = 0; i < GraphColoring::nnodes && j < GraphColoring::nnodes; i++)
	{
		if(j == pos)
			j = pos+tam;

		if(find(&p1[pos], &p1[pos+tam], p2[i]) == &p1[pos+tam])
			f[j++] = p2[i];
	}
	return;
}

// comparator function:
bool fnequal1(Problema* prob1, Problema* prob2)
{
	GraphColoring *p1 = dynamic_cast<GraphColoring *>(prob1);
	GraphColoring *p2 = dynamic_cast<GraphColoring *>(prob2);

	if(p1->sol.fitness == p2->sol.fitness)
	{
		for(int j = 0; j < GraphColoring::nnodes; j++)
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
	GraphColoring *p1 = dynamic_cast<GraphColoring *>(prob1);
	GraphColoring *p2 = dynamic_cast<GraphColoring *>(prob2);

	return p1->sol.fitness == p2->sol.fitness;
}

// comparator function:
bool fncomp1(Problema *prob1, Problema *prob2)
{
	GraphColoring *p1 = dynamic_cast<GraphColoring *>(prob1);
	GraphColoring *p2 = dynamic_cast<GraphColoring *>(prob2);

	if(p1->sol.fitness == p2->sol.fitness)
	{
		for(int j = 0; j < GraphColoring::nnodes; j++)
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
	GraphColoring *p1 = dynamic_cast<GraphColoring *>(prob1);
	GraphColoring *p2 = dynamic_cast<GraphColoring *>(prob2);

	return p1->sol.fitness < p2->sol.fitness;
}


inline bool ptcomp(pair<Problema*, InfoTabu*>* p1, pair<Problema*, InfoTabu*>* p2)
{
	return (p1->first->getFitness() > p2->first->getFitness());
}
