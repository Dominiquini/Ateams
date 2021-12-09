#include "BinPacking.hpp"

using namespace std;

/* Static Members */

ProblemType Problem::TIPO = MINIMIZACAO;

double Problem::best = 0;
double Problem::worst = 0;
int Problem::numInst = 0;
long int Problem::totalNumInst = 0;

char BinPacking::name[128];
double *BinPacking::sizes = NULL, BinPacking::capacity = 0;
int BinPacking::nitens = 0;

int BinPacking::num_vizinhos = 0;

Problem* Problem::randSoluction()
{
	return new BinPacking();
}

Problem* Problem::copySoluction(const Problem& prob)
{
	return new BinPacking(prob);
}


void Problem::readProblemFromFile(char* input)
{
	FILE *f = fopen(input, "r");

	if(f == NULL)
		throw "Wrong Data File!";

	if(!fgets (BinPacking::name, 128, f))
		throw "Wrong Data File!";

	if(!fscanf (f, "%lf %d %*d\n", &BinPacking::capacity, &BinPacking::nitens))
		throw "Wrong Data File!";

	Problem::alocaMemoria();

	for(int i = 0; i < BinPacking::nitens; i++)
	{
		if (!fscanf (f, "%lf", &BinPacking::sizes[i]))
			throw "Wrong Data File!";
	}

	for(int i = 1; i < BinPacking::nitens; i++)
		BinPacking::num_vizinhos += i;

	return;
}

list<Problem*>* Problem::readPopulationFromLog(char *log)
{
	FILE *f = fopen(log, "r");

	if(f != NULL)
	{
		list<Problem*>* popInicial = new list<Problem*>();
		double capacity;
		int npop, nitens, nbins;
		short int *ordem, *bins;
		Problem* p;

		if(!fscanf (f, "%d %lf %d", &npop, &capacity, &nitens))
			throw "Wrong Log File!";

		if(capacity != BinPacking::capacity || nitens != BinPacking::nitens)
			throw "Wrong Log File!";

		for(int i = 0; i < npop; i++)
		{
			ordem = (short int*)malloc(nitens * sizeof(short int));
			bins = (short int*)malloc(nitens * sizeof(short int));

			if(!fscanf (f, "%d", &nbins))
				throw "Wrong Log File!";

			for(int j = 0; j < nitens; j++)
			{
				if(!fscanf (f, "%hd %hd", &bins[j], &ordem[j]))
					throw "Wrong Log File!";
			}

			p = new BinPacking(ordem, bins);

			if(nbins != p->getFitness())
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
		short int *ordem, *bins;

		fprintf(f, "%d %f %d\n\n", sizePop, BinPacking::capacity, BinPacking::nitens);

		for(iter = popInicial->begin(); iter != popInicial->end(); iter++)
		{
			ordem = dynamic_cast<BinPacking *>(*iter)->getSoluction().ordemItens;
			bins = dynamic_cast<BinPacking *>(*iter)->getSoluction().bins;

			fprintf(f, "%d\n", (int)dynamic_cast<BinPacking *>(*iter)->getSoluction().fitness);

			for(int j = 0; j < BinPacking::nitens; j++)
			{
				fprintf(f, "%hd %hd ", bins[j], ordem[j]);
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
	BinPacking::sizes = (double*)malloc(BinPacking::nitens * sizeof(double));
}

void Problem::unloadMemory()
{
	free(BinPacking::sizes);
}


/* Metodos */

BinPacking::BinPacking() : Problem::Problem()
{
	sol.ordemItens = (short int*)malloc(nitens * sizeof(short int));

	if(Problem::totalNumInst == 1)	// Tenta uma solucao gulosa
	{
		double *orderSize = (double*)malloc(nitens * sizeof(double));

		for(int i = 0; i < nitens; i++)
			orderSize[i] = sizes[i];

		for(int i = 0, posMax = -1; i < nitens; i++)
		{
			double max = 0;

			for(int j = 0; j < nitens; j++)
			{
				if(orderSize[j] > max)
				{
					max = orderSize[j];
					posMax = j;
				}
			}

			orderSize[posMax] = -1;
			sol.ordemItens[i] = posMax;
		}

		double *tempValBins = (double*)malloc(nitens * sizeof(double));
		short int *tempBins = (short int*)malloc(nitens * sizeof(short int));

		for(int i = 0; i < nitens; i++)
		{
			orderSize[i] = sol.ordemItens[i];
			tempValBins[i] = -1;
		}

		int localFitness = 0;

		for(int i = 0; i < nitens; i++)
		{
			bool insert = false;

			for(int j = 0; j < localFitness; j++)
			{
				if(sizes[(int)orderSize[i]] + tempValBins[j] < capacity)
				{
					tempValBins[j] += sizes[(int)orderSize[i]];
					tempBins[(int)orderSize[i]] = j;
					insert = true;
					break;
				}
			}

			if(insert == false)
			{
				tempValBins[localFitness] = sizes[(int)orderSize[i]];
				tempBins[(int)orderSize[i]] = localFitness;
				localFitness++;
			}
		}

		for(int i = 0, bin = 0; i < localFitness; i++)
		{
			for(int j = 0; j < nitens; j++)
			{
				if(tempBins[j] == i)
				{
					sol.ordemItens[bin++] = j;
				}
			}
		}

		free(orderSize);
		free(tempValBins);
		free(tempBins);

		sol.bins = NULL;
		calcFitness(false);

		if((int)sol.fitness != localFitness)
		{
			cout << endl << endl << "Problema na Solução Gulosa!" << endl << endl;
			exit(1);
		}
	}
	else
	{
		for(int i = 0; i < nitens; i++)
			sol.ordemItens[i] = i;

		random_shuffle(&sol.ordemItens[0], &sol.ordemItens[nitens]);

		sol.bins = NULL;
		calcFitness(false);
	}

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}


BinPacking::BinPacking(short int *prob) : Problem::Problem()
{
	sol.ordemItens = prob;
	sol.bins = NULL;

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

BinPacking::BinPacking(short int *prob, short int *bins) : Problem::Problem()
{
	sol.ordemItens = prob;
	sol.bins = bins;

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

BinPacking::BinPacking(const Problem &prob) : Problem::Problem()
{
	BinPacking *other = dynamic_cast<BinPacking *>(const_cast<Problem *>(&prob));

	this->sol.ordemItens = (short int*)malloc(nitens * sizeof(short int));

	for(int i = 0; i < nitens; i++)
		this->sol.ordemItens[i] = other->sol.ordemItens[i];

	this->sol.bins = NULL;
	this->sol.fitness = other->sol.fitness;

	if(other->sol.bins != NULL)
	{
		this->sol.bins = (short int*)malloc(nitens * sizeof(short int));

		for(int i = 0; i < nitens; i++)
			this->sol.bins[i] = other->sol.bins[i];
	}
	exec = prob.exec;
}

BinPacking::BinPacking(const Problem &prob, int pos1, int pos2) : Problem::Problem()
{
	BinPacking *other = dynamic_cast<BinPacking *>(const_cast<Problem *>(&prob));

	this->sol.ordemItens = (short int*)malloc(nitens * sizeof(short int));

	for(int i = 0; i < nitens; i++)
		this->sol.ordemItens[i] = other->sol.ordemItens[i];

	short int aux = this->sol.ordemItens[pos1];
	this->sol.ordemItens[pos1] = this->sol.ordemItens[pos2];
	this->sol.ordemItens[pos2] = aux;

	this->sol.bins = NULL;

	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

BinPacking::~BinPacking()
{
	if(sol.ordemItens != NULL)
		free(sol.ordemItens);

	if(sol.bins != NULL)
		free(sol.bins);
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
inline bool BinPacking::calcFitness(bool esc)
{
	sol.fitness = 1;

	if(sol.bins == NULL)
	{
		double sumBinAtual = 0;
		int anterior = 0;

		short int *aux_bins = (short int*)malloc(nitens * sizeof(short int));

		for(int pos = 0; pos < nitens; pos++)
		{
			sumBinAtual += sizes[sol.ordemItens[pos]];

			if(sumBinAtual > capacity)
			{
				sol.fitness++;
				sumBinAtual = sizes[sol.ordemItens[pos]];

				sort(&sol.ordemItens[anterior], &sol.ordemItens[pos]);
				anterior = pos;
			}

			aux_bins[pos] = sol.fitness;
		}
		sort(&sol.ordemItens[anterior], &sol.ordemItens[nitens]);

		sol.bins = aux_bins;
	}
	else
	{
		sol.fitness = sol.bins[nitens - 1];
	}

	return true;
}

inline void BinPacking::imprimir(bool esc)
{
	if(esc == true)
	{
		calcFitness(esc);

		for(int i = 1; i <= sol.fitness; i++)
		{
			double sumBin = 0;
			int numItens = 0;

			printf("bin %.2d: ", i);
			for(int j = 0; j < nitens; j++)
			{
				if(sol.bins[j] == i)
				{
					printf("|%.4hd|", sol.ordemItens[j]+1);
					sumBin += sizes[sol.ordemItens[j]];
					numItens++;
				}
			}
			printf(" ==> %0.2f (%d)\n", sumBin, numItens);
		}
	}
	else
	{
		for(int j = 0; j < nitens; j++)
		{
			printf("%d ", sol.ordemItens[j]);
		}
	}
	printf("\n");
}

/* Retorna um vizinho aleatorio da solucao atual. */
inline Problem* BinPacking::vizinho()
{
	int p1 = xRand(0, nitens), p2 = xRand(0, nitens);
	Problem *prob = NULL;

	while(p2 == p1 || sol.bins[p1] == sol.bins[p2])
		p2 = xRand(0, nitens);

	prob = new BinPacking(*this, p1, p2);

	return prob;
}

/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual. */
inline vector<pair<Problem*, InfoTabu*>* >* BinPacking::buscaLocal()
{
	if(BinPacking::num_vizinhos > MAX_PERMUTACOES)
		return buscaLocal((float)MAX_PERMUTACOES/(float)BinPacking::num_vizinhos);

	Problem *prob = NULL;
	int p1, p2;
	pair<Problem*, InfoTabu*>* temp;
	vector<pair<Problem*, InfoTabu*>* >* local = new vector<pair<Problem*, InfoTabu*>* >();

	for(p1 = 0; p1 < nitens-1; p1++)
	{
		for(p2 = p1+1; p2 < nitens; p2++)
		{
			if(sol.bins[p1] != sol.bins[p2])
			{
				prob = new BinPacking(*this, p1, p2);

				temp = new pair<Problem*, InfoTabu*>();
				temp->first = prob;
				temp->second = new InfoTabu_BinPacking(p1, p2);

				local->push_back(temp);
			}
		}
	}

	random_shuffle(local->begin(), local->end());
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Retorna um conjunto de com uma parcela das solucoes viaveis vizinhas da atual. */
inline vector<pair<Problem*, InfoTabu*>* >* BinPacking::buscaLocal(float parcela)
{
	Problem *prob = NULL;
	int p1, p2;
	pair<Problem*, InfoTabu*>* temp;
	vector<pair<Problem*, InfoTabu*>* >* local = new vector<pair<Problem*, InfoTabu*>* >();
	int def;

	def = (int)((float)BinPacking::num_vizinhos*parcela);

	if(def > MAX_PERMUTACOES)
		def = MAX_PERMUTACOES;

	for(int i = 0; i < def; i++)
	{
		p1 = xRand(0, nitens), p2 = xRand(0, nitens);

		while(p2 == p1 || sol.bins[p1] == sol.bins[p2])
			p2 = xRand(0, nitens);

		prob = new BinPacking(*this, p1, p2);

		temp = new pair<Problem*, InfoTabu*>();
		temp->first = prob;
		temp->second = new InfoTabu_BinPacking(p1, p2);

		local->push_back(temp);
	}
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Realiza um crossover com uma outra solucao. Usa 2 pivos. */
inline pair<Problem*, Problem*>* BinPacking::crossOver(const Problem* parceiro, int tamParticao, int strength)
{
	short int *f1 = (short int*)malloc(nitens * sizeof(short int)), *f2 = (short int*)malloc(nitens * sizeof(short int));
	pair<Problem*, Problem*>* filhos = new pair<Problem*, Problem*>();
	int particao = tamParticao == 0 ? (BinPacking::nitens)/2 : tamParticao;
	int inicioPart = 0, fimPart = 0;

	BinPacking *other = dynamic_cast<BinPacking *>(const_cast<Problem *>(parceiro));

	inicioPart = xRand(0, nitens);
	fimPart = inicioPart+particao <= nitens ? inicioPart+particao : nitens;

	swap_vect(this->sol.ordemItens, other->sol.ordemItens, f1, inicioPart, fimPart-inicioPart);
	swap_vect(other->sol.ordemItens, this->sol.ordemItens, f2, inicioPart, fimPart-inicioPart);

	filhos->first = new BinPacking(f1);
	filhos->second = new BinPacking(f2);

	return filhos;
}

/* Realiza um crossover com uma outra solucao. Usa 1 pivo. */
inline pair<Problem*, Problem*>* BinPacking::crossOver(const Problem* parceiro, int strength)
{
	short int *f1 = (short int*)malloc(nitens * sizeof(short int)), *f2 = (short int*)malloc(nitens * sizeof(short int));
	pair<Problem*, Problem*>* filhos = new pair<Problem*, Problem*>();
	int particao = 0;

	BinPacking *other = dynamic_cast<BinPacking *>(const_cast<Problem *>(parceiro));

	particao = xRand(1, nitens);

	swap_vect(this->sol.ordemItens, other->sol.ordemItens, f1, 0, particao);
	swap_vect(other->sol.ordemItens, this->sol.ordemItens, f2, 0, particao);

	filhos->first = new BinPacking(f1);
	filhos->second = new BinPacking(f2);

	return filhos;
}

/* Devolve uma mutacao aleatoria na solucao atual. */
inline Problem* BinPacking::mutacao(int mutMax)
{
	short int *mut = (short int*)malloc(nitens * sizeof(short int));
	Problem* vizinho = NULL, *temp = NULL, *mutacao = NULL;

	for(int j = 0; j < nitens; j++)
		mut[j] = sol.ordemItens[j];

	temp = new BinPacking(mut);
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

inline double BinPacking::getFitness() const
{
	return sol.fitness;
}

inline double BinPacking::getFitnessMaximize() const
{
	return (double)INV_FITNESS/sol.fitness;
}

inline double BinPacking::getFitnessMinimize() const
{
	return (double)sol.fitness;
}

/* Auxiliares */

inline void swap_vect(short int* p1, short int* p2, short int* f, int pos, int tam)
{
	for(int i = pos; i < pos+tam; i++)
		f[i] = p1[i];

	for(int i = 0, j = 0; i < BinPacking::nitens && j < BinPacking::nitens; i++)
	{
		if(j == pos)
			j = pos+tam;

		if(find(&p1[pos], &p1[pos+tam], p2[i]) == &p1[pos+tam])
			f[j++] = p2[i];
	}
	return;
}

// comparator function:
bool fnequal1(Problem* prob1, Problem* prob2)
{
	BinPacking *p1 = dynamic_cast<BinPacking *>(prob1);
	BinPacking *p2 = dynamic_cast<BinPacking *>(prob2);

	if(p1->sol.fitness == p2->sol.fitness)
	{
		for(int j = 0; j < BinPacking::nitens; j++)
			if(p1->sol.ordemItens[j] != p2->sol.ordemItens[j])
				return false;

		return true;
	}
	else
		return false;
}

// comparator function:
bool fnequal2(Problem* prob1, Problem* prob2)
{
	BinPacking *p1 = dynamic_cast<BinPacking *>(prob1);
	BinPacking *p2 = dynamic_cast<BinPacking *>(prob2);

	return p1->sol.fitness == p2->sol.fitness;
}

// comparator function:
bool fncomp1(Problem *prob1, Problem *prob2)
{
	BinPacking *p1 = dynamic_cast<BinPacking *>(prob1);
	BinPacking *p2 = dynamic_cast<BinPacking *>(prob2);

	if(p1->sol.fitness == p2->sol.fitness)
	{
		for(int j = 0; j < BinPacking::nitens; j++)
			if(p1->sol.ordemItens[j] != p2->sol.ordemItens[j])
				return p1->sol.ordemItens[j] < p2->sol.ordemItens[j];

		return false;
	}
	else
		return p1->sol.fitness < p2->sol.fitness;
}

// comparator function:
bool fncomp2(Problem *prob1, Problem *prob2)
{
	BinPacking *p1 = dynamic_cast<BinPacking *>(prob1);
	BinPacking *p2 = dynamic_cast<BinPacking *>(prob2);

	return p1->sol.fitness < p2->sol.fitness;
}

inline bool ptcomp(pair<Problem*, InfoTabu*>* p1, pair<Problem*, InfoTabu*>* p2)
{
	return (p1->first->getFitness() > p2->first->getFitness());
}
