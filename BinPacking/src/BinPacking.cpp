#include "BinPacking.hpp"

using namespace std;

/* Static Members */

ProblemType Problem::TYPE = MINIMIZATION;

double Problem::best = 0;
double Problem::worst = 0;

unsigned int Problem::numInst = 0;
unsigned long long Problem::totalNumInst = 0;

unsigned int Problem::neighbors = 0;

char BinPacking::name[128];
double *BinPacking::sizes = NULL, BinPacking::capacity = 0;
int BinPacking::nitens = 0;

Problem* Problem::randomSolution() {
	return new BinPacking();
}

Problem* Problem::copySolution(const Problem &prob) {
	return new BinPacking(prob);
}

void Problem::readProblemFromFile(char *input) {
	FILE *f = fopen(input, "r");

	if (f == NULL)
		throw "Wrong Data File!";

	if (!fgets(BinPacking::name, 128, f))
		throw "Wrong Data File!";

	if (!fscanf(f, "%lf %d %*d\n", &BinPacking::capacity, &BinPacking::nitens))
		throw "Wrong Data File!";

	Problem::allocateMemory();

	for (int i = 0; i < BinPacking::nitens; i++) {
		if (!fscanf(f, "%lf", &BinPacking::sizes[i]))
			throw "Wrong Data File!";
	}

	for (int i = 1; i < BinPacking::nitens; i++)
		Problem::neighbors += i;

	fclose(f);

	return;
}

list<Problem*>* Problem::readPopulationFromLog(char *log) {
	FILE *f = fopen(log, "r");

	if (f != NULL) {
		list<Problem*> *popInicial = new list<Problem*>();
		double capacity;
		int npop, nitens, nbins;
		short int *ordem, *bins;
		BinPacking *bp;

		if (!fscanf(f, "%d %lf %d", &npop, &capacity, &nitens))
			throw "Wrong Log File!";

		if (capacity != BinPacking::capacity || nitens != BinPacking::nitens)
			throw "Wrong Log File!";

		for (int i = 0; i < npop; i++) {
			ordem = (short int*) allocateMatrix<short int>(1, nitens);
			bins = (short int*) allocateMatrix<short int>(1, nitens);

			if (!fscanf(f, "%d", &nbins))
				throw "Wrong Log File!";

			for (int j = 0; j < nitens; j++) {
				if (!fscanf(f, "%hd %hd", &bins[j], &ordem[j]))
					throw "Wrong Log File!";
			}

			bp = new BinPacking(ordem);

			if (nbins != bp->getFitness())
				throw "Wrong Log File!";

			for (int j = 0; j < nitens; j++) {
				if (bins[j] != bp->getSoluction().bins[j])
					throw "Wrong Log File!";
			}

			popInicial->push_back(bp);
		}

		fclose(f);

		return popInicial;
	} else {
		return NULL;
	}
}

void Problem::writeCurrentPopulationInLog(char *log, list<Problem*> *initialPopulation) {
	FILE *f = fopen(log, "w");

	if (f != NULL) {
		int sizePop = (int) initialPopulation->size();
		list<Problem*>::iterator iter;
		short int *ordem, *bins;

		fprintf(f, "%d %f %d\n\n", sizePop, BinPacking::capacity, BinPacking::nitens);

		for (iter = initialPopulation->begin(); iter != initialPopulation->end(); iter++) {
			BinPacking *bp = dynamic_cast<BinPacking*>(*iter);

			ordem = bp->getSoluction().ordemItens;
			bins = bp->getSoluction().bins;

			fprintf(f, "%d\n", (int) bp->getSoluction().fitness);

			for (int j = 0; j < BinPacking::nitens; j++) {
				fprintf(f, "%hd %hd ", bins[j], ordem[j]);
			}

			fprintf(f, "\n\n");
		}

		fclose(f);
	}
}

void Problem::writeResultInFile(char *input, char *parameters, char *result, ExecutionInfo *info) {
	FILE *f;

	if (*result != '\0') {
		if ((f = fopen(result, "r+")) != NULL) {
			fseek(f, 0, SEEK_END);
		} else {
			f = fopen(result, "w");

			fprintf(f, "%*s%*s", -16, "bestFitness", -16, "worstFitness");
			fprintf(f, "%*s%*s%*s", -16, "executions", -16, "diffTime", -24, "expSol");
			fprintf(f, "%*s%s\n", -24, "input", "parameters");
		}

		fprintf(f, "%*d%*d", -16, (int) info->bestFitness, -16, (int) info->worstFitness);
		fprintf(f, "%*d%*d%*d", -16, (int) info->executionCount, -16, (int) info->executionTime.count(), -24, (int) info->exploredSolutions);
		fprintf(f, "%*s%s\n", -24, FileUtils::getFileName(string(input)).c_str(), FileUtils::getFileName(string(parameters)).c_str());

		fclose(f);
	}
}

void Problem::allocateMemory() {
	Problem::best = 0;
	Problem::worst = 0;

	Problem::numInst = 0;
	Problem::totalNumInst = 0;

	Problem::neighbors = 0;

	BinPacking::sizes = (double*) malloc(BinPacking::nitens * sizeof(double));
}

void Problem::deallocateMemory() {
	Problem::best = 0;
	Problem::worst = 0;

	Problem::numInst = 0;
	Problem::totalNumInst = 0;

	Problem::neighbors = 0;

	free(BinPacking::sizes);
}

/* Metodos */

BinPacking::BinPacking() : Problem::Problem() {
	solution.ordemItens = (short int*) allocateMatrix<short int>(1, nitens);

	if (Problem::totalNumInst == 1) { // Tenta uma solucao gulosa
		double *orderSize = (double*) allocateMatrix<double>(1, nitens);

		for (int i = 0; i < nitens; i++)
			orderSize[i] = sizes[i];

		for (int i = 0, posMax = -1; i < nitens; i++) {
			double max = 0;

			for (int j = 0; j < nitens; j++) {
				if (orderSize[j] > max) {
					max = orderSize[j];
					posMax = j;
				}
			}

			orderSize[posMax] = -1;
			solution.ordemItens[i] = posMax;
		}

		double *tempValBins = (double*) allocateMatrix<double>(1, nitens);
		short int *tempBins = (short int*) allocateMatrix<short int>(1, nitens);

		for (int i = 0; i < nitens; i++) {
			orderSize[i] = solution.ordemItens[i];
			tempValBins[i] = -1;
		}

		int localFitness = 0;

		for (int i = 0; i < nitens; i++) {
			bool insert = false;

			for (int j = 0; j < localFitness; j++) {
				if (sizes[(int) orderSize[i]] + tempValBins[j] < capacity) {
					tempValBins[j] += sizes[(int) orderSize[i]];
					tempBins[(int) orderSize[i]] = j;
					insert = true;
					break;
				}
			}

			if (insert == false) {
				tempValBins[localFitness] = sizes[(int) orderSize[i]];
				tempBins[(int) orderSize[i]] = localFitness;
				localFitness++;
			}
		}

		for (int i = 0, bin = 0; i < localFitness; i++) {
			for (int j = 0; j < nitens; j++) {
				if (tempBins[j] == i) {
					solution.ordemItens[bin++] = j;
				}
			}
		}

		deallocateMatrix<double>(1, orderSize);
		deallocateMatrix<double>(1, tempValBins);
		deallocateMatrix<short int>(1, tempBins);
	} else {
		for (int i = 0; i < nitens; i++) {
			solution.ordemItens[i] = i;
		}

		shuffle(&solution.ordemItens[0], &solution.ordemItens[nitens], randomEngine);
	}

	solution.bins = NULL;

	calcFitness();
}

BinPacking::BinPacking(short int *prob) : Problem::Problem() {
	solution.ordemItens = prob;
	solution.bins = NULL;

	calcFitness();
}

BinPacking::BinPacking(const Problem &prob) : Problem::Problem() {
	BinPacking *other = dynamic_cast<BinPacking*>(const_cast<Problem*>(&prob));

	this->solution.ordemItens = (short int*) allocateMatrix<short int>(1, nitens);

	for (int i = 0; i < nitens; i++)
		this->solution.ordemItens[i] = other->solution.ordemItens[i];

	this->solution.bins = NULL;
	this->solution.fitness = other->solution.fitness;

	if (other->solution.bins != NULL) {
		this->solution.bins = (short int*) allocateMatrix<short int>(1, nitens);

		for (int i = 0; i < nitens; i++)
			this->solution.bins[i] = other->solution.bins[i];
	}
}

BinPacking::BinPacking(const Problem &prob, int pos1, int pos2) : Problem::Problem() {
	BinPacking *other = dynamic_cast<BinPacking*>(const_cast<Problem*>(&prob));

	this->solution.ordemItens = (short int*) allocateMatrix<short int>(1, nitens);

	for (int i = 0; i < nitens; i++)
		this->solution.ordemItens[i] = other->solution.ordemItens[i];

	short int aux = this->solution.ordemItens[pos1];
	this->solution.ordemItens[pos1] = this->solution.ordemItens[pos2];
	this->solution.ordemItens[pos2] = aux;

	this->solution.bins = NULL;

	calcFitness();
}

BinPacking::~BinPacking() {
	deallocateMatrix<short int>(1, solution.ordemItens);

	deallocateMatrix<short int>(1, solution.bins);
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
inline bool BinPacking::calcFitness() {
	deallocateMatrix<short int>(1, solution.bins);

	double sumBinAtual = 0;
	int bins = 1;

	short int *aux_bins = (short int*) allocateMatrix<short int>(1, nitens);

	for (int pos = 0; pos < nitens; pos++) {
		sumBinAtual += sizes[solution.ordemItens[pos]];

		if (sumBinAtual > capacity) {
			bins++;
			sumBinAtual = sizes[solution.ordemItens[pos]];
		}

		aux_bins[pos] = bins;
	}

	solution.bins = aux_bins;
	solution.fitness = bins;

	return true;
}

inline void BinPacking::print(bool esc) {
	if (esc == true) {
		calcFitness();

		for (int i = 1; i <= solution.fitness; i++) {
			double size = 0;
			double sumBin = 0;
			int numItens = 0;

			printf("bin %.2d: ", i);
			for (int j = 0; j < nitens; j++) {
				if (solution.bins[j] == i) {
					size = sizes[solution.ordemItens[j]];
					sumBin += size;
					numItens++;

					printf("|%.2f (%.4hd)|", size, solution.ordemItens[j] + 1);
				}
			}
			printf(" ==> %.2f (%d)\n", sumBin, numItens);
		}
	} else {
		for (int j = 0; j < nitens; j++) {
			printf("%d ", solution.ordemItens[j]);
		}
		printf("\n");
	}
}

/* Retorna um vizinho aleatorio da solucao atual. */
inline Problem* BinPacking::neighbor() {
	int p1 = Random::randomNumber(0, nitens), p2 = Random::randomNumber(0, nitens);
	Problem *prob = NULL;

	while (p2 == p1 || solution.bins[p1] == solution.bins[p2])
		p2 = Random::randomNumber(0, nitens);

	prob = new BinPacking(*this, p1, p2);

	return prob;
}

/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual. */
inline vector<pair<Problem*, InfoTabu*>*>* BinPacking::localSearch() {
	if (Problem::neighbors > MAX_PERMUTATIONS)
		return localSearch((float) MAX_PERMUTATIONS / (float) Problem::neighbors);

	Problem *prob = NULL;
	int p1, p2;
	pair<Problem*, InfoTabu*> *temp;
	vector<pair<Problem*, InfoTabu*>*> *local = new vector<pair<Problem*, InfoTabu*>*>();

	for (p1 = 0; p1 < nitens - 1; p1++) {
		for (p2 = p1 + 1; p2 < nitens; p2++) {
			if (solution.bins[p1] != solution.bins[p2]) {
				prob = new BinPacking(*this, p1, p2);

				temp = new pair<Problem*, InfoTabu*>();
				temp->first = prob;
				temp->second = new InfoTabu_BinPacking(p1, p2);

				local->push_back(temp);
			}
		}
	}

	shuffle(local->begin(), local->end(), randomEngine);
	sort(local->begin(), local->end(), Problem::ptcomp);

	return local;
}

/* Retorna um conjunto de uma parcela das solucoes viaveis vizinhas da atual. */
inline vector<pair<Problem*, InfoTabu*>*>* BinPacking::localSearch(float parcela) {
	Problem *prob = NULL;
	int p1, p2;
	pair<Problem*, InfoTabu*> *temp;
	vector<pair<Problem*, InfoTabu*>*> *local = new vector<pair<Problem*, InfoTabu*>*>();
	int def;

	def = (int) ((float) Problem::neighbors * parcela);

	if (def > MAX_PERMUTATIONS)
		def = MAX_PERMUTATIONS;

	for (int i = 0; i < def; i++) {
		p1 = Random::randomNumber(0, nitens), p2 = Random::randomNumber(0, nitens);

		while (p2 == p1 || solution.bins[p1] == solution.bins[p2])
			p2 = Random::randomNumber(0, nitens);

		prob = new BinPacking(*this, p1, p2);

		temp = new pair<Problem*, InfoTabu*>();
		temp->first = prob;
		temp->second = new InfoTabu_BinPacking(p1, p2);

		local->push_back(temp);
	}

	sort(local->begin(), local->end(), Problem::ptcomp);

	return local;
}

/* Realiza um crossover com uma outra solucao. Usa 2 pivos. */
inline pair<Problem*, Problem*>* BinPacking::crossOver(const Problem *parceiro, int partitionSize, int strength) {
	short int *f1 = (short int*) allocateMatrix<short int>(1, nitens), *f2 = (short int*) allocateMatrix<short int>(1, nitens);
	pair<Problem*, Problem*> *filhos = new pair<Problem*, Problem*>();
	int particao = partitionSize == 0 ? (BinPacking::nitens) / 2 : partitionSize;
	int inicioPart = 0, fimPart = 0;

	BinPacking *other = dynamic_cast<BinPacking*>(const_cast<Problem*>(parceiro));

	inicioPart = Random::randomNumber(0, nitens);
	fimPart = inicioPart + particao <= nitens ? inicioPart + particao : nitens;

	swap_vect(this->solution.ordemItens, other->solution.ordemItens, f1, inicioPart, fimPart - inicioPart);
	swap_vect(other->solution.ordemItens, this->solution.ordemItens, f2, inicioPart, fimPart - inicioPart);

	filhos->first = new BinPacking(f1);
	filhos->second = new BinPacking(f2);

	return filhos;
}

/* Realiza um crossover com uma outra solucao. Usa 1 pivo. */
inline pair<Problem*, Problem*>* BinPacking::crossOver(const Problem *parceiro, int strength) {
	short int *f1 = (short int*) allocateMatrix<short int>(1, nitens), *f2 = (short int*) allocateMatrix<short int>(1, nitens);
	pair<Problem*, Problem*> *filhos = new pair<Problem*, Problem*>();
	int particao = 0;

	BinPacking *other = dynamic_cast<BinPacking*>(const_cast<Problem*>(parceiro));

	particao = Random::randomNumber(1, nitens);

	swap_vect(this->solution.ordemItens, other->solution.ordemItens, f1, 0, particao);
	swap_vect(other->solution.ordemItens, this->solution.ordemItens, f2, 0, particao);

	filhos->first = new BinPacking(f1);
	filhos->second = new BinPacking(f2);

	return filhos;
}

/* Devolve uma mutacao aleatoria na solucao atual. */
inline Problem* BinPacking::mutation(int mutMax) {
	short int *mut = (short int*) allocateMatrix<short int>(1, nitens);
	Problem *vizinho = NULL, *temp = NULL, *mutacao = NULL;

	for (int j = 0; j < nitens; j++)
		mut[j] = solution.ordemItens[j];

	temp = new BinPacking(mut);
	mutacao = temp;

	while (mutMax-- > 0) {
		vizinho = temp->neighbor();

		delete temp;
		temp = vizinho;
		mutacao = temp;
	}
	return mutacao;
}

inline double BinPacking::getFitness() const {
	return solution.fitness;
}

inline double BinPacking::getFitnessMaximize() const {
	return (double) INV_FITNESS / solution.fitness;
}

inline double BinPacking::getFitnessMinimize() const {
	return (double) solution.fitness;
}

/* Auxiliares */

inline void swap_vect(short int *p1, short int *p2, short int *f, int pos, int tam) {
	for (int i = pos; i < pos + tam; i++)
		f[i] = p1[i];

	for (int i = 0, j = 0; i < BinPacking::nitens && j < BinPacking::nitens; i++) {
		if (j == pos)
			j = pos + tam;

		if (find(&p1[pos], &p1[pos + tam], p2[i]) == &p1[pos + tam])
			f[j++] = p2[i];
	}
	return;
}

// comparator function:
bool fnEqualSolution(Problem *prob1, Problem *prob2) {
	BinPacking *p1 = dynamic_cast<BinPacking*>(prob1);
	BinPacking *p2 = dynamic_cast<BinPacking*>(prob2);

	if (p1->solution.fitness == p2->solution.fitness) {
		for (int j = 0; j < BinPacking::nitens; j++)
			if (p1->solution.ordemItens[j] != p2->solution.ordemItens[j])
				return false;

		return true;
	} else
		return false;
}

// comparator function:
bool fnEqualFitness(Problem *prob1, Problem *prob2) {
	BinPacking *p1 = dynamic_cast<BinPacking*>(prob1);
	BinPacking *p2 = dynamic_cast<BinPacking*>(prob2);

	return p1->solution.fitness == p2->solution.fitness;
}

// comparator function:
bool fnSortSolution(Problem *prob1, Problem *prob2) {
	BinPacking *p1 = dynamic_cast<BinPacking*>(prob1);
	BinPacking *p2 = dynamic_cast<BinPacking*>(prob2);

	if (p1->solution.fitness == p2->solution.fitness) {
		for (int j = 0; j < BinPacking::nitens; j++)
			if (p1->solution.ordemItens[j] != p2->solution.ordemItens[j])
				return p1->solution.ordemItens[j] < p2->solution.ordemItens[j];

		return false;
	} else
		return p1->solution.fitness < p2->solution.fitness;
}

// comparator function:
bool fnSortFitness(Problem *prob1, Problem *prob2) {
	BinPacking *p1 = dynamic_cast<BinPacking*>(prob1);
	BinPacking *p2 = dynamic_cast<BinPacking*>(prob2);

	return p1->solution.fitness < p2->solution.fitness;
}
