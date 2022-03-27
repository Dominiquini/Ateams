#include "TravellingSalesman.hpp"

using namespace std;

/* Static Members */

ProblemType Problem::TYPE = MINIMIZATION;

double Problem::best = 0;
double Problem::worst = 0;

unsigned int Problem::numInst = 0;
unsigned long long Problem::totalNumInst = 0;

unsigned int Problem::neighbors = 0;

char TravellingSalesman::name[128];
double **TravellingSalesman::edges = NULL;
int TravellingSalesman::nnodes = 0;

Problem* Problem::randomSolution() {
	return new TravellingSalesman();
}

Problem* Problem::copySolution(const Problem &prob) {
	return new TravellingSalesman(prob);
}

void Problem::readProblemFromFile(char *input) {
	FILE *f = fopen(input, "r");

	if (f == NULL)
		throw "Wrong Data File!";

	char edge_weight_type[32];
	char edge_weight_format[32];

	char line[128];
	while (fgets(line, 128, f)) {
		if (strstr(line, "NAME: "))
			sscanf(line + strlen("NAME: "), "%s", TravellingSalesman::name);

		if (strstr(line, "DIMENSION: "))
			sscanf(line + strlen("DIMENSION: "), "%d", &TravellingSalesman::nnodes);

		if (strstr(line, "EDGE_WEIGHT_TYPE: "))
			sscanf(line + strlen("EDGE_WEIGHT_TYPE: "), "%s", edge_weight_type);

		if (strstr(line, "EDGE_WEIGHT_FORMAT: "))
			sscanf(line + strlen("EDGE_WEIGHT_FORMAT: "), "%s", edge_weight_format);

		if (strstr(line, "EDGE_WEIGHT_SECTION") || strstr(line, "NODE_COORD_SECTION"))
			break;
	}

	Problem::allocateMemory();

	if (!strcmp(edge_weight_type, "EXPLICIT")) {
		if (!strcmp(edge_weight_format, "FULL_MATRIX")) {
			for (int i = 0; i < TravellingSalesman::nnodes; i++) {
				for (int j = 0; j < TravellingSalesman::nnodes; j++) {
					if (!fscanf(f, "%lf", &TravellingSalesman::edges[i][j]))
						throw "Wrong Data File!";

					if (i == j)
						TravellingSalesman::edges[i][j] = -1;
				}
			}
		}

		if (!strcmp(edge_weight_format, "UPPER_ROW")) {
			for (int i = 0; i < TravellingSalesman::nnodes; i++) {
				TravellingSalesman::edges[i][i] = -1;

				for (int j = i + 1; j < TravellingSalesman::nnodes; j++) {
					if (!fscanf(f, "%lf", &TravellingSalesman::edges[i][j]))
						throw "Wrong Data File!";

					TravellingSalesman::edges[j][i] = TravellingSalesman::edges[i][j];
				}
			}
		}

		if (!strcmp(edge_weight_format, "UPPER_DIAG_ROW")) {
			for (int i = 0; i < TravellingSalesman::nnodes; i++) {
				for (int j = i; j < TravellingSalesman::nnodes; j++) {
					if (!fscanf(f, "%lf", &TravellingSalesman::edges[i][j]))
						throw "Wrong Data File!";

					TravellingSalesman::edges[j][i] = TravellingSalesman::edges[i][j];

					if (i == j)
						TravellingSalesman::edges[i][j] = -1;
				}
			}
		}

		if (!strcmp(edge_weight_format, "LOWER_ROW")) {
			for (int i = 0; i < TravellingSalesman::nnodes; i++) {
				TravellingSalesman::edges[i][i] = -1;

				for (int j = 0; j < i; j++) {
					if (!fscanf(f, "%lf", &TravellingSalesman::edges[i][j]))
						throw "Wrong Data File!";

					TravellingSalesman::edges[j][i] = TravellingSalesman::edges[i][j];
				}
			}
		}

		if (!strcmp(edge_weight_format, "LOWER_DIAG_ROW")) {
			for (int i = 0; i < TravellingSalesman::nnodes; i++) {
				for (int j = 0; j <= i; j++) {
					if (!fscanf(f, "%lf", &TravellingSalesman::edges[i][j]))
						throw "Wrong Data File!";

					TravellingSalesman::edges[j][i] = TravellingSalesman::edges[i][j];

					if (i == j)
						TravellingSalesman::edges[i][j] = -1;
				}
			}
		}
	} else if (!strcmp(edge_weight_type, "EUC_2D")) {
		double *X = (double*) allocateMatrix<double>(1, TravellingSalesman::nnodes);
		double *Y = (double*) allocateMatrix<double>(1, TravellingSalesman::nnodes);
		int no = 0;

		for (int i = 0; i < TravellingSalesman::nnodes; i++) {
			if (!fscanf(f, "%d %lf %lf", &no, &X[i], &Y[i]))
				throw "Wrong Data File!";

			if (i + 1 != no)
				throw "Wrong Data File!";
		}

		double xd, yd;

		for (int i = 0; i < TravellingSalesman::nnodes; i++) {
			for (int j = 0; j < TravellingSalesman::nnodes; j++) {
				xd = X[i] - X[j];
				yd = Y[i] - Y[j];

				TravellingSalesman::edges[i][j] = sqrt(pow(xd, 2) + pow(yd, 2));
			}
		}

		free(X);
		free(Y);
	}

	for (int i = 1; i <= TravellingSalesman::nnodes; i++)
		Problem::neighbors += i;

	fclose(f);

	return;
}

list<Problem*>* Problem::readPopulationFromLog(char *log) {
	FILE *f = fopen(log, "r");

	if (f != NULL) {
		list<Problem*> *popInicial = new list<Problem*>();
		double peso;
		int npop, nnodes;
		short int *ordem;
		TravellingSalesman *ts;

		if (!fscanf(f, "%d %d", &npop, &nnodes))
			throw "Wrong Log File!";

		if (nnodes != TravellingSalesman::nnodes)
			throw "Wrong Log File!";

		for (int i = 0; i < npop; i++) {
			ordem = (short int*) allocateMatrix<short int>(1, nnodes + 1);
			;

			if (!fscanf(f, "%lf", &peso))
				throw "Wrong Log File!";

			for (int j = 0; j <= nnodes; j++) {
				if (!fscanf(f, "%hd", &ordem[j]))
					throw "Wrong Log File!";
			}

			ts = new TravellingSalesman(ordem);

			if (peso != ts->getFitness())
				throw "Wrong Log File!";

			popInicial->push_back(ts);
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
		short int *ordem;

		fprintf(f, "%d %d\n\n", sizePop, TravellingSalesman::nnodes);

		for (iter = initialPopulation->begin(); iter != initialPopulation->end(); iter++) {
			TravellingSalesman *ts = dynamic_cast<TravellingSalesman*>(*iter);

			ordem = ts->getSoluction().ordemNodes;

			fprintf(f, "%d\n", (int) ts->getSoluction().fitness);

			for (int j = 0; j <= TravellingSalesman::nnodes; j++) {
				fprintf(f, "%hd ", ordem[j]);
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

	TravellingSalesman::edges = (double**) malloc(TravellingSalesman::nnodes * sizeof(double*));

	for (int i = 0; i < TravellingSalesman::nnodes; i++)
		TravellingSalesman::edges[i] = (double*) malloc(TravellingSalesman::nnodes * sizeof(double));
}

void Problem::deallocateMemory() {
	Problem::best = 0;
	Problem::worst = 0;

	Problem::numInst = 0;
	Problem::totalNumInst = 0;

	Problem::neighbors = 0;

	for (int i = 0; i < TravellingSalesman::nnodes; i++)
		free(TravellingSalesman::edges[i]);

	free(TravellingSalesman::edges);
}

/* Metodos */

TravellingSalesman::TravellingSalesman() : Problem::Problem() {
	solution.ordemNodes = (short int*) allocateMatrix<short int>(1, nnodes + 1);

	if (Random::randomPercentage() <= 20) { // Tenta uma solucao gulosa
		int currentNode = Random::randomNumber(0, nnodes);
		int nextNode = 0;
		int position = 0;
		double d = 0;

		solution.ordemNodes[position++] = currentNode;
		for (int i = 0; i < nnodes; i++) {
			nextNode = -1;
			d = INT_MAX;

			for (int j = 0; j < nnodes && i != nnodes - 1; j++) {
				if (edges[currentNode][j] != -1 && edges[currentNode][j] < d && find(&solution.ordemNodes[0], &solution.ordemNodes[position], j) == &solution.ordemNodes[position]) {
					nextNode = j;
					d = edges[currentNode][nextNode];
				}
			}

			if (nextNode == -1) {
				if (i == nnodes - 1)
					nextNode = solution.ordemNodes[0];
				else
					return;
			}

			solution.ordemNodes[position++] = nextNode;
			currentNode = nextNode;
		}
	} else {
		for (int i = 0; i < nnodes; i++) {
			solution.ordemNodes[i] = i;
		}

		random_shuffle(&solution.ordemNodes[0], &solution.ordemNodes[nnodes], as_lambda(Random::randomNumber));
		solution.ordemNodes[nnodes] = solution.ordemNodes[0];
	}

	calcFitness();
}

TravellingSalesman::TravellingSalesman(short int *prob) : Problem::Problem() {
	solution.ordemNodes = prob;

	calcFitness();
}

TravellingSalesman::TravellingSalesman(const Problem &prob) : Problem::Problem() {
	TravellingSalesman *other = dynamic_cast<TravellingSalesman*>(const_cast<Problem*>(&prob));

	this->solution.ordemNodes = (short int*) allocateMatrix<short int>(1, nnodes + 1);

	for (int i = 0; i <= nnodes; i++)
		this->solution.ordemNodes[i] = other->solution.ordemNodes[i];

	this->solution.fitness = other->solution.fitness;
}

TravellingSalesman::TravellingSalesman(const Problem &prob, int pos1, int pos2) : Problem::Problem() {
	TravellingSalesman *other = dynamic_cast<TravellingSalesman*>(const_cast<Problem*>(&prob));

	this->solution.ordemNodes = (short int*) allocateMatrix<short int>(1, nnodes + 1);

	for (int i = 0; i <= nnodes; i++)
		this->solution.ordemNodes[i] = other->solution.ordemNodes[i];

	short int aux = this->solution.ordemNodes[pos1];
	this->solution.ordemNodes[pos1] = this->solution.ordemNodes[pos2];
	this->solution.ordemNodes[pos2] = aux;

	if (pos1 == 0 || pos2 == 0)
		solution.ordemNodes[nnodes] = solution.ordemNodes[0];
	else if (pos1 == nnodes || pos2 == nnodes)
		solution.ordemNodes[0] = solution.ordemNodes[nnodes];

	calcFitness();
}

TravellingSalesman::~TravellingSalesman() {
	deallocateMatrix<short int>(1, solution.ordemNodes);
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
inline bool TravellingSalesman::calcFitness() {
	double sumEdges = 0;

	double d;
	int c1, c2;
	for (int c = 0; c < nnodes; c++) {
		c1 = solution.ordemNodes[c];
		c2 = solution.ordemNodes[c + 1];

		if ((d = edges[c1][c2]) != -1)
			sumEdges += d;
		else
			return false;
	}

	solution.fitness = sumEdges;

	return true;
}

inline void TravellingSalesman::print(bool esc) {
	if (esc == false) {
		for (int j = 0; j <= nnodes; j++) {
			printf("%d ", solution.ordemNodes[j]);
		}

		printf("\n");
	} else {
		printf("|--> ");
		for (int j = 0; j < nnodes; j++) {
			printf("%d -> ", solution.ordemNodes[j] + 1);
		}
		printf("%d |\n", solution.ordemNodes[nnodes] + 1);
	}
}

/* Retorna um vizinho aleatorio da solucao atual. */
inline Problem* TravellingSalesman::neighbor() {
	int p1 = Random::randomNumber(0, nnodes + 1), p2 = Random::randomNumber(0, nnodes + 1);
	Problem *prob = NULL;

	while (p2 == p1)
		p2 = Random::randomNumber(0, nnodes + 1);

	prob = new TravellingSalesman(*this, p1, p2);
	if (prob->getFitness() != -1) {
		return prob;
	} else {
		delete prob;
		return NULL;
	}
}

/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual. */
inline vector<pair<Problem*, InfoTabu*>*>* TravellingSalesman::localSearch() {
	if (Problem::neighbors > MAX_PERMUTATIONS)
		return localSearch((float) MAX_PERMUTATIONS / (float) Problem::neighbors);

	Problem *prob = NULL;
	int p1, p2;
	pair<Problem*, InfoTabu*> *temp;
	vector<pair<Problem*, InfoTabu*>*> *local = new vector<pair<Problem*, InfoTabu*>*>();

	for (p1 = 0; p1 < nnodes; p1++) {
		for (p2 = p1 + 1; p2 <= nnodes; p2++) {
			prob = new TravellingSalesman(*this, p1, p2);

			if (prob->getFitness() != -1) {
				temp = new pair<Problem*, InfoTabu*>();
				temp->first = prob;
				temp->second = new InfoTabu_TravellingSalesman(p1, p2);

				local->push_back(temp);
			} else {
				delete prob;
			}
		}
	}

	random_shuffle(local->begin(), local->end(), as_lambda(Random::randomNumber));
	sort(local->begin(), local->end(), Problem::ptcomp);

	return local;
}

/* Retorna um conjunto de uma parcela das solucoes viaveis vizinhas da atual. */
inline vector<pair<Problem*, InfoTabu*>*>* TravellingSalesman::localSearch(float parcela) {
	Problem *prob = NULL;
	int p1, p2;
	pair<Problem*, InfoTabu*> *temp;
	vector<pair<Problem*, InfoTabu*>*> *local = new vector<pair<Problem*, InfoTabu*>*>();
	int def;

	def = (int) ((float) Problem::neighbors * parcela);

	if (def > MAX_PERMUTATIONS)
		def = MAX_PERMUTATIONS;

	for (int i = 0; i < def; i++) {
		p1 = Random::randomNumber(0, nnodes + 1), p2 = Random::randomNumber(0, nnodes + 1);

		while (p2 == p1)
			p2 = Random::randomNumber(0, nnodes + 1);

		prob = new TravellingSalesman(*this, p1, p2);
		if (prob->getFitness() != -1) {
			temp = new pair<Problem*, InfoTabu*>();
			temp->first = prob;
			temp->second = new InfoTabu_TravellingSalesman(p1, p2);

			local->push_back(temp);
		} else {
			delete prob;
		}
	}

	sort(local->begin(), local->end(), Problem::ptcomp);

	return local;
}

/* Realiza um crossover com uma outra solucao. Usa 2 pivos. */
inline pair<Problem*, Problem*>* TravellingSalesman::crossOver(const Problem *parceiro, int partitionSize, int strength) {
	short int *f1 = (short int*) allocateMatrix<short int>(1, nnodes + 1), *f2 = (short int*) allocateMatrix<short int>(1, nnodes + 1);
	pair<Problem*, Problem*> *filhos = new pair<Problem*, Problem*>();
	int particao = partitionSize == 0 ? (TravellingSalesman::nnodes) / 2 : partitionSize;
	int inicioPart = 0, fimPart = 0;

	TravellingSalesman *other = dynamic_cast<TravellingSalesman*>(const_cast<Problem*>(parceiro));

	inicioPart = Random::randomNumber(0, nnodes);
	fimPart = inicioPart + particao <= nnodes ? inicioPart + particao : nnodes;

	swap_vect(this->solution.ordemNodes, other->solution.ordemNodes, f1, inicioPart, fimPart - inicioPart);
	swap_vect(other->solution.ordemNodes, this->solution.ordemNodes, f2, inicioPart, fimPart - inicioPart);

	filhos->first = new TravellingSalesman(f1);
	filhos->second = new TravellingSalesman(f2);

	return filhos;
}

/* Realiza um crossover com uma outra solucao. Usa 1 pivo. */
inline pair<Problem*, Problem*>* TravellingSalesman::crossOver(const Problem *parceiro, int strength) {
	short int *f1 = (short int*) allocateMatrix<short int>(1, nnodes + 1), *f2 = (short int*) allocateMatrix<short int>(1, nnodes + 1);
	pair<Problem*, Problem*> *filhos = new pair<Problem*, Problem*>();
	int particao = 0;

	TravellingSalesman *other = dynamic_cast<TravellingSalesman*>(const_cast<Problem*>(parceiro));

	particao = Random::randomNumber(1, nnodes);

	swap_vect(this->solution.ordemNodes, other->solution.ordemNodes, f1, 0, particao);
	swap_vect(other->solution.ordemNodes, this->solution.ordemNodes, f2, 0, particao);

	filhos->first = new TravellingSalesman(f1);
	filhos->second = new TravellingSalesman(f2);

	return filhos;
}

/* Devolve uma mutacao aleatoria na solucao atual. */
inline Problem* TravellingSalesman::mutation(int mutMax) {
	short int *mut = (short int*) allocateMatrix<short int>(1, nnodes + 1);
	Problem *vizinho = NULL, *temp = NULL, *mutacao = NULL;

	for (int j = 0; j <= nnodes; j++)
		mut[j] = solution.ordemNodes[j];

	temp = new TravellingSalesman(mut);
	mutacao = temp;

	while (mutMax-- > 0) {
		vizinho = temp->neighbor();

		if (vizinho != NULL) {
			delete temp;
			temp = vizinho;
			mutacao = temp;
		}
	}
	return mutacao;
}

inline double TravellingSalesman::getFitness() const {
	return solution.fitness;
}

inline double TravellingSalesman::getFitnessMaximize() const {
	return (double) INV_FITNESS / solution.fitness;
}

inline double TravellingSalesman::getFitnessMinimize() const {
	return (double) solution.fitness;
}

/* Auxiliares */

inline void swap_vect(short int *p1, short int *p2, short int *f, int pos, int tam) {
	for (int i = pos; i < pos + tam; i++)
		f[i] = p1[i];

	for (int i = 0, j = 0; i < TravellingSalesman::nnodes && j < TravellingSalesman::nnodes; i++) {
		if (j == pos)
			j = pos + tam;

		if (find(&p1[pos], &p1[pos + tam], p2[i]) == &p1[pos + tam])
			f[j++] = p2[i];
	}

	f[TravellingSalesman::nnodes] = f[0];

	return;
}

// comparator function:
bool fnEqualSolution(Problem *prob1, Problem *prob2) {
	TravellingSalesman *p1 = dynamic_cast<TravellingSalesman*>(prob1);
	TravellingSalesman *p2 = dynamic_cast<TravellingSalesman*>(prob2);

	if (p1->solution.fitness == p2->solution.fitness) {
		for (int j = 0; j <= TravellingSalesman::nnodes; j++)
			if (p1->solution.ordemNodes[j] != p2->solution.ordemNodes[j])
				return false;

		return true;
	} else
		return false;
}

// comparator function:
bool fnEqualFitness(Problem *prob1, Problem *prob2) {
	TravellingSalesman *p1 = dynamic_cast<TravellingSalesman*>(prob1);
	TravellingSalesman *p2 = dynamic_cast<TravellingSalesman*>(prob2);

	return p1->solution.fitness == p2->solution.fitness;
}

// comparator function:
bool fnSortSolution(Problem *prob1, Problem *prob2) {
	TravellingSalesman *p1 = dynamic_cast<TravellingSalesman*>(prob1);
	TravellingSalesman *p2 = dynamic_cast<TravellingSalesman*>(prob2);

	if (p1->solution.fitness == p2->solution.fitness) {
		for (int j = 0; j <= TravellingSalesman::nnodes; j++)
			if (p1->solution.ordemNodes[j] != p2->solution.ordemNodes[j])
				return p1->solution.ordemNodes[j] < p2->solution.ordemNodes[j];

		return false;
	} else
		return p1->solution.fitness < p2->solution.fitness;
}

// comparator function:
bool fnSortFitness(Problem *prob1, Problem *prob2) {
	TravellingSalesman *p1 = dynamic_cast<TravellingSalesman*>(prob1);
	TravellingSalesman *p2 = dynamic_cast<TravellingSalesman*>(prob2);

	return p1->solution.fitness < p2->solution.fitness;
}
