#include "KnapSack.hpp"

using namespace std;

/* Static Members */

ProblemType Problem::TYPE = MAXIMIZATION;

double Problem::best = 0;
double Problem::worst = 0;

unsigned int Problem::numInst = 0;
unsigned long long Problem::totalNumInst = 0;

unsigned int Problem::neighbors = 0;

char KnapSack::name[128];
double *KnapSack::values = NULL, **KnapSack::constraint = NULL, *KnapSack::limit = NULL;
int KnapSack::nitens = 0, KnapSack::ncontraints = 0;

Problem* Problem::randomSolution() {
	return new KnapSack();
}

Problem* Problem::copySolution(const Problem &prob) {
	return new KnapSack(prob);
}

void Problem::readProblemFromFile(char *input) {
	FILE *f = fopen(input, "r");

	if (f == NULL)
		throw "Wrong Data File!";

	if (!fgets(KnapSack::name, 128, f))
		throw "Wrong Data File!";

	if (!fscanf(f, "%d %d %*d\n", &KnapSack::nitens, &KnapSack::ncontraints))
		throw "Wrong Data File!";

	Problem::allocateMemory();

	for (int i = 0; i < KnapSack::nitens; i++) {
		if (!fscanf(f, "%lf", &KnapSack::values[i]))
			throw "Wrong Data File!";
	}

	double **constraints = (double**) allocateMatrix<double>(2, KnapSack::ncontraints, KnapSack::nitens);

	for (int i = 0; i < KnapSack::ncontraints; i++) {
		for (int j = 0; j < KnapSack::nitens; j++) {
			if (!fscanf(f, "%lf", &constraints[i][j]))
				throw "Wrong Data File!";
		}
	}

	for (int i = 0; i < KnapSack::nitens; i++)
		for (int j = 0; j < KnapSack::ncontraints; j++)
			KnapSack::constraint[i][j] = constraints[j][i];

	deallocateMatrix<double>(2, constraints, KnapSack::ncontraints);

	for (int i = 0; i < KnapSack::ncontraints; i++) {
		if (!fscanf(f, "%lf", &KnapSack::limit[i]))
			throw "Wrong Data File!";
	}

	for (int i = 1; i < KnapSack::nitens + 2; i++)
		Problem::neighbors += i;

	Problem::neighbors /= 2;

	fclose(f);

	return;
}

list<Problem*>* Problem::readPopulationFromLog(char *log) {
	FILE *f = fopen(log, "r");

	if (f != NULL) {
		list<Problem*> *popInicial = new list<Problem*>();
		int npop, nitens, nconstraint, valorTotal, limit;
		char format_type[32];
		short int *prob;
		KnapSack *ks;

		if (!fscanf(f, "%d %d %d %s\n", &npop, &nitens, &nconstraint, format_type))
			throw "Wrong Log File!";

		if (nitens != KnapSack::nitens || nconstraint != KnapSack::ncontraints)
			throw "Wrong Log File!";

		for (int s = 0; s < npop; s++) {
			prob = (short int*) allocateMatrix<short int>(1, nitens);

			if (!fscanf(f, "%d\n", &valorTotal))
				throw "Wrong Log File!";

			if (!fscanf(f, "%d ", &limit))
				throw "Wrong Log File!";

			for (int i = 0; i < nitens; i++) {
				if (!fscanf(f, "%hd ", &prob[i]))
					throw "Wrong Log File!";
			}

			ks = new KnapSack(prob);

			if (valorTotal != ks->getFitness())
				throw "Wrong Log File!";

			if (limit != ks->getSoluction().limit)
				throw "Wrong Log File!";

			popInicial->push_back(ks);
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
		short int *prob;

		fprintf(f, "%d %d %d %s\n\n", sizePop, KnapSack::nitens, KnapSack::ncontraints, "src_1");

		for (iter = initialPopulation->begin(); iter != initialPopulation->end(); iter++) {
			KnapSack *ks = dynamic_cast<KnapSack*>(*iter);

			prob = ks->getSoluction().ordemItens;

			fprintf(f, "%d\n", (int) ks->getSoluction().fitness);
			fprintf(f, "%d ", (int) ks->getSoluction().limit);
			for (int i = 0; i < KnapSack::nitens; i++) {
				fprintf(f, "%d ", prob[i]);
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

	KnapSack::values = (double*) malloc(KnapSack::nitens * sizeof(double));

	KnapSack::limit = (double*) malloc(KnapSack::ncontraints * sizeof(double));

	KnapSack::constraint = (double**) malloc(KnapSack::nitens * sizeof(double*));

	for (int i = 0; i < KnapSack::nitens; i++)
		KnapSack::constraint[i] = (double*) malloc(KnapSack::ncontraints * sizeof(double));
}

void Problem::deallocateMemory() {
	Problem::best = 0;
	Problem::worst = 0;

	Problem::numInst = 0;
	Problem::totalNumInst = 0;

	Problem::neighbors = 0;

	free(KnapSack::values);

	free(KnapSack::limit);

	for (int i = 0; i < KnapSack::nitens; i++)
		free(KnapSack::constraint[i]);

	free(KnapSack::constraint);
}

/* Metodos */

KnapSack::KnapSack() : Problem::Problem() {
	solution.ordemItens = (short int*) allocateMatrix<short int>(1, nitens);

	for (int i = 0; i < nitens; i++) {
		solution.ordemItens[i] = i;
	}

	shuffle(&solution.ordemItens[0], &solution.ordemItens[nitens], randomEngine);

	solution.limit = -1;

	calcFitness();
}

KnapSack::KnapSack(short int *prob) : Problem::Problem() {
	solution.ordemItens = prob;
	solution.limit = -1;

	calcFitness();
}

KnapSack::KnapSack(const Problem &prob) : Problem::Problem() {
	KnapSack *other = dynamic_cast<KnapSack*>(const_cast<Problem*>(&prob));

	this->solution.ordemItens = (short int*) allocateMatrix<short int>(1, nitens);
	for (int i = 0; i < nitens; i++)
		this->solution.ordemItens[i] = other->solution.ordemItens[i];

	this->solution.limit = other->solution.limit;

	this->solution.fitness = other->solution.fitness;
}

KnapSack::KnapSack(const Problem &prob, int pos1, int pos2) : Problem::Problem() {
	KnapSack *other = dynamic_cast<KnapSack*>(const_cast<Problem*>(&prob));

	this->solution.ordemItens = (short int*) allocateMatrix<short int>(1, nitens);
	for (int i = 0; i < nitens; i++)
		this->solution.ordemItens[i] = other->solution.ordemItens[i];

	short int aux = this->solution.ordemItens[pos1];
	this->solution.ordemItens[pos1] = this->solution.ordemItens[pos2];
	this->solution.ordemItens[pos2] = aux;

	solution.limit = -1;

	calcFitness();
}

KnapSack::~KnapSack() {
	deallocateMatrix<short int>(1, solution.ordemItens);
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
inline bool KnapSack::calcFitness() {
	vector<double> tempConstraints(ncontraints, 0);

	double fitness = 0;
	int item = 0, limit = 0;
	for (limit = 0; limit < nitens; limit++) {
		item = solution.ordemItens[limit];

		bool overflow = false;

		int c = 0;
		for (vector<double>::iterator constraint = tempConstraints.begin(); constraint != tempConstraints.end(); constraint++) {
			*constraint += KnapSack::constraint[item][c];

			if (*constraint > KnapSack::limit[c]) {
				overflow = true;
				break;
			}

			c++;
		}

		if (overflow) {
			break;
		}

		fitness += values[item];
	}

	sort(&solution.ordemItens[0], &solution.ordemItens[limit]);
	sort(&solution.ordemItens[limit], &solution.ordemItens[nitens]);

	solution.fitness = fitness;
	solution.limit = limit;

	return true;
}

inline void KnapSack::print(bool esc) {
	if (esc == true) {
		double sumItens = 0;
		int numItens = 0;

		printf("+sack: ");
		for (int i = 0; i < solution.limit; i++) {
			printf("|%.4d|", solution.ordemItens[i] + 1);
			sumItens += values[solution.ordemItens[i]];
			numItens++;
		}

		printf(" ==> %0.2f (%d)\n", sumItens, numItens);

		sumItens = 0;
		numItens = 0;

		printf("-sack: ");
		for (int i = solution.limit; i < nitens; i++) {
			printf("|%.4d|", solution.ordemItens[i] + 1);
			sumItens += values[solution.ordemItens[i]];
			numItens++;
		}

		printf(" ==> %0.2f (%d)\n", sumItens, numItens);
	} else {
		for (int i = 0; i < nitens; i++)
			printf("%d ", solution.ordemItens[i]);
	}
}

/* Retorna um vizinho aleatorio da solucao atual. */
inline Problem* KnapSack::neighbor() {
	int p1 = Random::randomNumber(0, solution.limit), p2 = Random::randomNumber(solution.limit, nitens);
	Problem *prob = NULL;

	prob = new KnapSack(*this, p1, p2);

	return prob;
}

/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual. */
inline vector<pair<Problem*, InfoTabu*>*>* KnapSack::localSearch() {
	if (Problem::neighbors > MAX_PERMUTATIONS)
		return localSearch((float) MAX_PERMUTATIONS / (float) Problem::neighbors);

	Problem *prob = NULL;
	int p1, p2;
	pair<Problem*, InfoTabu*> *temp;
	vector<pair<Problem*, InfoTabu*>*> *local = new vector<pair<Problem*, InfoTabu*>*>();

	for (p1 = 0; p1 < solution.limit; p1++) {
		for (p2 = solution.limit; p2 < nitens; p2++) {
			prob = new KnapSack(*this, p1, p2);

			temp = new pair<Problem*, InfoTabu*>();
			temp->first = prob;
			temp->second = new InfoTabu_KnapSack(p1, p2);

			local->push_back(temp);
		}
	}

	shuffle(local->begin(), local->end(), randomEngine);
	sort(local->begin(), local->end(), Problem::ptcomp);

	return local;
}

/* Retorna um conjunto de uma parcela das solucoes viaveis vizinhas da atual. */
inline vector<pair<Problem*, InfoTabu*>*>* KnapSack::localSearch(float parcela) {
	Problem *prob = NULL;
	int p1, p2;
	int numItens = nitens;
	pair<Problem*, InfoTabu*> *temp;
	vector<pair<Problem*, InfoTabu*>*> *local = new vector<pair<Problem*, InfoTabu*>*>();
	int def;

	def = (int) ((float) Problem::neighbors * parcela);

	if (def > MAX_PERMUTATIONS)
		def = MAX_PERMUTATIONS;

	for (int i = 0; i < def; i++) {
		p1 = Random::randomNumber(0, solution.limit), p2 = Random::randomNumber(solution.limit, numItens);

		prob = new KnapSack(*this, p1, p2);

		temp = new pair<Problem*, InfoTabu*>();
		temp->first = prob;
		temp->second = new InfoTabu_KnapSack(p1, p2);

		local->push_back(temp);
	}

	sort(local->begin(), local->end(), Problem::ptcomp);

	return local;
}

/* Realiza um crossover com uma outra solucao. Usa 2 pivos. */
inline pair<Problem*, Problem*>* KnapSack::crossOver(const Problem *parceiro, int partitionSize, int strength) {
	short int *f1 = (short int*) allocateMatrix<short int>(1, nitens), *f2 = (short int*) allocateMatrix<short int>(1, nitens);
	pair<Problem*, Problem*> *filhos = new pair<Problem*, Problem*>();
	int particao = partitionSize == 0 ? (nitens) / 2 : partitionSize;
	int inicioPart = 0, fimPart = 0;

	KnapSack *other = dynamic_cast<KnapSack*>(const_cast<Problem*>(parceiro));

	inicioPart = Random::randomNumber(0, nitens);
	fimPart = inicioPart + particao <= nitens ? inicioPart + particao : nitens;

	swap_vect(this->solution.ordemItens, other->solution.ordemItens, f1, inicioPart, fimPart - inicioPart);
	swap_vect(other->solution.ordemItens, this->solution.ordemItens, f2, inicioPart, fimPart - inicioPart);

	filhos->first = new KnapSack(f1);
	filhos->second = new KnapSack(f2);

	return filhos;
}

/* Realiza um crossover com uma outra solucao. Usa 1 pivo. */
inline pair<Problem*, Problem*>* KnapSack::crossOver(const Problem *parceiro, int strength) {
	short int *f1 = (short int*) allocateMatrix<short int>(1, nitens), *f2 = (short int*) allocateMatrix<short int>(1, nitens);
	pair<Problem*, Problem*> *filhos = new pair<Problem*, Problem*>();
	int particao = 0;

	KnapSack *other = dynamic_cast<KnapSack*>(const_cast<Problem*>(parceiro));

	particao = Random::randomNumber(1, nitens);

	swap_vect(this->solution.ordemItens, other->solution.ordemItens, f1, 0, particao);
	swap_vect(other->solution.ordemItens, this->solution.ordemItens, f2, 0, particao);

	filhos->first = new KnapSack(f1);
	filhos->second = new KnapSack(f2);

	return filhos;
}

/* Devolve uma mutacao aleatoria na solucao atual. */
inline Problem* KnapSack::mutation(int mutMax) {
	short int *mut = (short int*) allocateMatrix<short int>(1, nitens);
	Problem *vizinho = NULL, *temp = NULL, *mutacao = NULL;

	for (int i = 0; i < nitens; i++)
		mut[i] = solution.ordemItens[i];

	temp = new KnapSack(mut);
	mutacao = temp;

	while (mutMax-- > 0) {
		vizinho = temp->neighbor();

		delete temp;
		temp = vizinho;
		mutacao = temp;
	}

	return mutacao;
}

inline double KnapSack::getFitness() const {
	return solution.fitness;
}

inline double KnapSack::getFitnessMaximize() const {
	return (double) solution.fitness;
}

inline double KnapSack::getFitnessMinimize() const {
	return (double) INV_FITNESS / solution.fitness;
}

/* Auxiliares */

inline void swap_vect(short int *p1, short int *p2, short int *f, int pos, int tam) {
	for (int i = pos; i < pos + tam; i++)
		f[i] = p1[i];

	for (int i = 0, j = 0; i < KnapSack::nitens && j < KnapSack::nitens; i++) {
		if (j == pos)
			j = pos + tam;

		if (find(&p1[pos], &p1[pos + tam], p2[i]) == &p1[pos + tam])
			f[j++] = p2[i];
	}
	return;
}

// comparator function:
bool fnEqualSolution(Problem *prob1, Problem *prob2) {
	KnapSack *p1 = dynamic_cast<KnapSack*>(const_cast<Problem*>(prob1));
	KnapSack *p2 = dynamic_cast<KnapSack*>(const_cast<Problem*>(prob2));

	if (p1->solution.fitness == p2->solution.fitness) {
		for (int i = 0; i < KnapSack::nitens; i++)
			if (p1->solution.ordemItens[i] != p2->solution.ordemItens[i])
				return false;

		return true;
	} else
		return false;
}

// comparator function:
bool fnEqualFitness(Problem *prob1, Problem *prob2) {
	KnapSack *p1 = dynamic_cast<KnapSack*>(const_cast<Problem*>(prob1));
	KnapSack *p2 = dynamic_cast<KnapSack*>(const_cast<Problem*>(prob2));

	return p1->solution.fitness == p2->solution.fitness;
}

// comparator function:
bool fnSortSolution(Problem *prob1, Problem *prob2) {
	KnapSack *p1 = dynamic_cast<KnapSack*>(const_cast<Problem*>(prob1));
	KnapSack *p2 = dynamic_cast<KnapSack*>(const_cast<Problem*>(prob2));

	if (p1->solution.fitness == p2->solution.fitness) {
		for (int i = 0; i < KnapSack::nitens; i++)
			if (p1->solution.ordemItens[i] != p2->solution.ordemItens[i])
				return p1->solution.ordemItens[i] > p2->solution.ordemItens[i];

		return false;
	} else
		return p1->solution.fitness > p2->solution.fitness;
}

// comparator function:
bool fnSortFitness(Problem *prob1, Problem *prob2) {
	KnapSack *p1 = dynamic_cast<KnapSack*>(const_cast<Problem*>(prob1));
	KnapSack *p2 = dynamic_cast<KnapSack*>(const_cast<Problem*>(prob2));

	return p1->solution.fitness > p2->solution.fitness;
}
