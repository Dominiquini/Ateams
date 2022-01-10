#include "GraphColoring.hpp"

using namespace std;

/* Static Members */

ProblemType Problem::TYPE = MINIMIZATION;

double Problem::best = 0;
double Problem::worst = 0;

unsigned int Problem::numInst = 0;
unsigned long long Problem::totalNumInst = 0;

unsigned int Problem::neighbors = 0;

char GraphColoring::name[128];
vector<int> **GraphColoring::edges = NULL;
int GraphColoring::nedges = 0, GraphColoring::nnodes = 0;

Problem* Problem::randomSolution() {
	return new GraphColoring();
}

Problem* Problem::copySolution(const Problem &prob) {
	return new GraphColoring(prob);
}

void Problem::readProblemFromFile(char *input) {
	FILE *f = fopen(input, "r");

	if (f == NULL)
		throw "Wrong Data File!";

	char tempLine[256];
	int n1, n2;

	while (fgets(tempLine, 128, f)) {
		if (tempLine[0] == 'p') {
			sscanf(tempLine, "%*c %s %d %d\n", GraphColoring::name, &GraphColoring::nnodes, &GraphColoring::nedges);
		}

		Problem::allocateMemory();

		if (tempLine[0] == 'e') {
			sscanf(tempLine, "%*c %d %d", &n1, &n2);

			GraphColoring::edges[n1]->push_back(n2);
			GraphColoring::edges[n2]->push_back(n1);
		}
	}

	for (int i = 1; i < GraphColoring::nnodes; i++)
		Problem::neighbors += i;

	fclose(f);

	return;
}

list<Problem*>* Problem::readPopulationFromLog(char *log) {
	FILE *f = fopen(log, "r");

	if (f != NULL) {
		list<Problem*> *popInicial = new list<Problem*>();
		int npop, nnodes, nedges, ncolors;
		short int *ordem;
		GraphColoring *gc;

		if (!fscanf(f, "%d %d %d", &npop, &nnodes, &nedges))
			throw "Wrong Log File!";

		if (nnodes != GraphColoring::nnodes || nedges != GraphColoring::nedges)
			throw "Wrong Log File!";

		for (int i = 0; i < npop; i++) {
			ordem = (short int*) allocateMatrix<short int>(1, nnodes, 1, 1);

			if (!fscanf(f, "%d", &ncolors))
				throw "Wrong Log File!";

			for (int j = 0; j < nnodes; j++) {
				if (!fscanf(f, "%hd", &ordem[j]))
					throw "Wrong Log File!";
			}

			gc = new GraphColoring(ordem);

			if (ncolors != gc->getFitness())
				throw "Wrong Log File!";

			popInicial->push_back(gc);
		}

		fclose(f);

		return popInicial;
	} else {
		return NULL;
	}
}

void Problem::writeCurrentPopulationInLog(char *log, list<Problem*> *popInicial) {
	FILE *f = fopen(log, "w");

	if (f != NULL) {
		int sizePop = (int) popInicial->size();
		list<Problem*>::iterator iter;
		short int *ordem;

		fprintf(f, "%d %d %d\n\n", sizePop, GraphColoring::nnodes, GraphColoring::nedges);

		for (iter = popInicial->begin(); iter != popInicial->end(); iter++) {
			GraphColoring *gc = dynamic_cast<GraphColoring*>(*iter);

			ordem = gc->getSoluction().ordemNodes;

			fprintf(f, "%d\n", (int) gc->getSoluction().fitness);

			for (int j = 0; j < GraphColoring::nnodes; j++) {
				fprintf(f, "%hd ", ordem[j]);
			}

			fprintf(f, "\n\n");
		}

		fclose(f);
	}
}

void Problem::writeResultInFile(char *dados, char *parametros, ExecutionInfo info, char *resultado) {
	FILE *f;

	if (*resultado != '\0') {
		if ((f = fopen(resultado, "r+")) != NULL) {
			fseek(f, 0, SEEK_END);
		} else {
			f = fopen(resultado, "w");

			fprintf(f, "%*s%*s", -16, "bestFitness", -16, "worstFitness");
			fprintf(f, "%*s%*s%*s", -16, "numExecs", -16, "diffTime", -24, "expSol");
			fprintf(f, "%*s%s\n", -24, "input", "parameters");
		}

		fprintf(f, "%*d%*d", -16, (int) info.bestFitness, -16, (int) info.worstFitness);
		fprintf(f, "%*d%*d%*d", -16, (int) info.executionCount, -16, (int) info.executionTime, -24, (int) info.exploredSolutions);
		fprintf(f, "%*s%s\n", -24, dados, parametros);

		fclose(f);
	}
}

void Problem::allocateMemory() {
	Problem::best = 0;
	Problem::worst = 0;

	Problem::numInst = 0;
	Problem::totalNumInst = 0;

	Problem::neighbors = 0;

	GraphColoring::edges = (vector<int>**) malloc((1 + GraphColoring::nnodes) * sizeof(vector<int>*));

	for (int i = 1; i <= GraphColoring::nnodes; i++)
		GraphColoring::edges[i] = new vector<int>();
}

void Problem::deallocateMemory() {
	Problem::best = 0;
	Problem::worst = 0;

	Problem::numInst = 0;
	Problem::totalNumInst = 0;

	Problem::neighbors = 0;

	for (int i = 1; i <= GraphColoring::nnodes; i++)
		delete GraphColoring::edges[i];

	free(GraphColoring::edges);
}

/* Metodos */

GraphColoring::GraphColoring() : Problem::Problem() {
	solution.ordemNodes = (short int*) allocateMatrix<short int>(1, nnodes, 1, 1);

	for (int i = 0; i < nnodes; i++) {
		solution.ordemNodes[i] = i + 1;
	}

	random_shuffle(&solution.ordemNodes[0], &solution.ordemNodes[nnodes], pointer_to_unary_function<int, int>(randomNumber));

	solution.colors = NULL;

	calcFitness();
}

GraphColoring::GraphColoring(short int *prob) : Problem::Problem() {
	solution.ordemNodes = prob;

	solution.colors = NULL;

	calcFitness();
}

GraphColoring::GraphColoring(const Problem &prob) : Problem::Problem() {
	GraphColoring *other = dynamic_cast<GraphColoring*>(const_cast<Problem*>(&prob));

	this->solution.ordemNodes = (short int*) allocateMatrix<short int>(1, nnodes, 1, 1);

	for (int i = 0; i < nnodes; i++)
		this->solution.ordemNodes[i] = other->solution.ordemNodes[i];

	this->solution.colors = NULL;
	this->solution.fitness = other->solution.fitness;

	if (other->solution.colors != NULL) {
		this->solution.colors = (short int*) allocateMatrix<short int>(1, nnodes + 1, 1, 1);

		for (int i = 0; i <= nnodes; i++)
			this->solution.colors[i] = other->solution.colors[i];
	}
}

GraphColoring::GraphColoring(const Problem &prob, int pos1, int pos2) : Problem::Problem() {
	GraphColoring *other = dynamic_cast<GraphColoring*>(const_cast<Problem*>(&prob));

	this->solution.ordemNodes = (short int*) allocateMatrix<short int>(1, nnodes, 1, 1);

	for (int i = 0; i < nnodes; i++)
		this->solution.ordemNodes[i] = other->solution.ordemNodes[i];

	short int aux = this->solution.ordemNodes[pos1];
	this->solution.ordemNodes[pos1] = this->solution.ordemNodes[pos2];
	this->solution.ordemNodes[pos2] = aux;

	this->solution.colors = NULL;

	calcFitness();
}

GraphColoring::~GraphColoring() {
	deallocateMatrix<short int>(1, solution.ordemNodes, nnodes, 1);

	deallocateMatrix<short int>(1, solution.colors, nnodes + 1, 1);
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
inline bool GraphColoring::calcFitness() {
	deallocateMatrix<short int>(1, solution.colors, nnodes + 1, 1);

	short int *aux_colors = (short int*) allocateMatrix<short int>(1, nnodes + 1, 1, 1);

	for (int i = 1; i <= nnodes; i++)
		aux_colors[i] = 0;

	int minColor = 1;
	aux_colors[0] = 1;

	int noCorrente, corCorrente;
	vector<int>::iterator vizinhos;
	for (int n = 0; n < nnodes; n++) {
		noCorrente = solution.ordemNodes[n];
		corCorrente = -1;

		for (int c = 1; c <= minColor; c++) {
			for (vizinhos = edges[noCorrente]->begin(); vizinhos != edges[noCorrente]->end() && aux_colors[*vizinhos] != c; vizinhos++);

			if (vizinhos == edges[noCorrente]->end()) {
				corCorrente = c;
				break;
			}
		}

		if (corCorrente < 0)
			corCorrente = ++minColor;

		aux_colors[noCorrente] = corCorrente;
	}

	aux_colors[0] = minColor;

	solution.colors = aux_colors;
	solution.fitness = minColor;

	return true;
}

inline void GraphColoring::print(bool esc) {
	if (esc == true) {
		calcFitness();

		for (int i = 1; i <= solution.fitness; i++) {
			printf("color %.2d: ", i);
			for (int j = 1; j <= nnodes; j++) {
				if (solution.colors[j] == i)
					printf("|%d|", j);
			}
			printf("\n");
		}
	} else {
		for (int j = 0; j < nnodes; j++) {
			printf("%d ", solution.ordemNodes[j]);
		}
	}
}

/* Retorna um vizinho aleatorio da solucao atual. */
inline Problem* GraphColoring::neighbor() {
	int p1 = randomNumber(0, nnodes), p2 = randomNumber(0, nnodes);
	Problem *prob = NULL;

	while (p2 == p1)
		p2 = randomNumber(0, nnodes);

	prob = new GraphColoring(*this, p1, p2);

	return prob;
}

/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual. */
inline vector<pair<Problem*, InfoTabu*>*>* GraphColoring::localSearch() {
	if (Problem::neighbors > MAX_PERMUTATIONS)
		return localSearch((float) MAX_PERMUTATIONS / (float) Problem::neighbors);

	Problem *prob = NULL;
	int p1, p2;
	pair<Problem*, InfoTabu*> *temp;
	vector<pair<Problem*, InfoTabu*>*> *local = new vector<pair<Problem*, InfoTabu*>*>();

	for (p1 = 0; p1 < nnodes - 1; p1++) {
		for (p2 = p1 + 1; p2 < nnodes; p2++) {
			prob = new GraphColoring(*this, p1, p2);

			temp = new pair<Problem*, InfoTabu*>();
			temp->first = prob;
			temp->second = new InfoTabu_GraphColoring(p1, p2);

			local->push_back(temp);
		}
	}

	random_shuffle(local->begin(), local->end(), pointer_to_unary_function<int, int>(randomNumber));
	sort(local->begin(), local->end(), Problem::ptcomp);

	return local;
}

/* Retorna um conjunto de uma parcela das solucoes viaveis vizinhas da atual. */
inline vector<pair<Problem*, InfoTabu*>*>* GraphColoring::localSearch(float parcela) {
	Problem *prob = NULL;
	int p1, p2;
	pair<Problem*, InfoTabu*> *temp;
	vector<pair<Problem*, InfoTabu*>*> *local = new vector<pair<Problem*, InfoTabu*>*>();
	int def;

	def = (int) ((float) Problem::neighbors * parcela);

	if (def > MAX_PERMUTATIONS)
		def = MAX_PERMUTATIONS;

	for (int i = 0; i < def; i++) {
		p1 = randomNumber(0, nnodes), p2 = randomNumber(0, nnodes);

		while (p2 == p1)
			p2 = randomNumber(0, nnodes);

		prob = new GraphColoring(*this, p1, p2);

		temp = new pair<Problem*, InfoTabu*>();
		temp->first = prob;
		temp->second = new InfoTabu_GraphColoring(p1, p2);

		local->push_back(temp);
	}

	sort(local->begin(), local->end(), Problem::ptcomp);

	return local;
}

/* Realiza um crossover com uma outra solucao. Usa 2 pivos. */
inline pair<Problem*, Problem*>* GraphColoring::crossOver(const Problem *parceiro, int partitionSize, int strength) {
	short int *f1 = (short int*) allocateMatrix<short int>(1, nnodes, 1, 1), *f2 = (short int*) allocateMatrix<short int>(1, nnodes, 1, 1);
	pair<Problem*, Problem*> *filhos = new pair<Problem*, Problem*>();
	int particao = partitionSize == 0 ? (GraphColoring::nnodes) / 2 : partitionSize;
	int inicioPart = 0, fimPart = 0;

	GraphColoring *other = dynamic_cast<GraphColoring*>(const_cast<Problem*>(parceiro));

	inicioPart = randomNumber(0, nnodes);
	fimPart = inicioPart + particao <= nnodes ? inicioPart + particao : nnodes;

	swap_vect(this->solution.ordemNodes, other->solution.ordemNodes, f1, inicioPart, fimPart - inicioPart);
	swap_vect(other->solution.ordemNodes, this->solution.ordemNodes, f2, inicioPart, fimPart - inicioPart);

	filhos->first = new GraphColoring(f1);
	filhos->second = new GraphColoring(f2);

	return filhos;
}

/* Realiza um crossover com uma outra solucao. Usa 1 pivo. */
inline pair<Problem*, Problem*>* GraphColoring::crossOver(const Problem *parceiro, int strength) {
	short int *f1 = (short int*) allocateMatrix<short int>(1, nnodes, 1, 1), *f2 = (short int*) allocateMatrix<short int>(1, nnodes, 1, 1);
	pair<Problem*, Problem*> *filhos = new pair<Problem*, Problem*>();
	int particao = 0;

	GraphColoring *other = dynamic_cast<GraphColoring*>(const_cast<Problem*>(parceiro));

	particao = randomNumber(1, nnodes);

	swap_vect(this->solution.ordemNodes, other->solution.ordemNodes, f1, 0, particao);
	swap_vect(other->solution.ordemNodes, this->solution.ordemNodes, f2, 0, particao);

	filhos->first = new GraphColoring(f1);
	filhos->second = new GraphColoring(f2);

	return filhos;
}

/* Devolve uma mutacao aleatoria na solucao atual. */
inline Problem* GraphColoring::mutation(int mutMax) {
	short int *mut = (short int*) allocateMatrix<short int>(1, nnodes, 1, 1);
	Problem *vizinho = NULL, *temp = NULL, *mutacao = NULL;

	for (int j = 0; j < nnodes; j++)
		mut[j] = solution.ordemNodes[j];

	temp = new GraphColoring(mut);
	mutacao = temp;

	while (mutMax-- > 0) {
		vizinho = temp->neighbor();

		delete temp;
		temp = vizinho;
		mutacao = temp;
	}
	return mutacao;
}

inline double GraphColoring::getFitness() const {
	return solution.fitness;
}

inline double GraphColoring::getFitnessMaximize() const {
	return (double) INV_FITNESS / solution.fitness;
}

inline double GraphColoring::getFitnessMinimize() const {
	return (double) solution.fitness;
}

/* Auxiliares */

inline void swap_vect(short int *p1, short int *p2, short int *f, int pos, int tam) {
	for (int i = pos; i < pos + tam; i++)
		f[i] = p1[i];

	for (int i = 0, j = 0; i < GraphColoring::nnodes && j < GraphColoring::nnodes; i++) {
		if (j == pos)
			j = pos + tam;

		if (find(&p1[pos], &p1[pos + tam], p2[i]) == &p1[pos + tam])
			f[j++] = p2[i];
	}
	return;
}

// comparator function:
bool fnEqualSolution(Problem *prob1, Problem *prob2) {
	GraphColoring *p1 = dynamic_cast<GraphColoring*>(prob1);
	GraphColoring *p2 = dynamic_cast<GraphColoring*>(prob2);

	if (p1->solution.fitness == p2->solution.fitness) {
		for (int j = 0; j < GraphColoring::nnodes; j++)
			if (p1->solution.ordemNodes[j] != p2->solution.ordemNodes[j])
				return false;

		return true;
	} else
		return false;
}

// comparator function:
bool fnEqualFitness(Problem *prob1, Problem *prob2) {
	GraphColoring *p1 = dynamic_cast<GraphColoring*>(prob1);
	GraphColoring *p2 = dynamic_cast<GraphColoring*>(prob2);

	return p1->solution.fitness == p2->solution.fitness;
}

// comparator function:
bool fnSortSolution(Problem *prob1, Problem *prob2) {
	GraphColoring *p1 = dynamic_cast<GraphColoring*>(prob1);
	GraphColoring *p2 = dynamic_cast<GraphColoring*>(prob2);

	if (p1->solution.fitness == p2->solution.fitness) {
		for (int j = 0; j < GraphColoring::nnodes; j++)
			if (p1->solution.ordemNodes[j] != p2->solution.ordemNodes[j])
				return p1->solution.ordemNodes[j] < p2->solution.ordemNodes[j];

		return false;
	} else
		return p1->solution.fitness < p2->solution.fitness;
}

// comparator function:
bool fnSortFitness(Problem *prob1, Problem *prob2) {
	GraphColoring *p1 = dynamic_cast<GraphColoring*>(prob1);
	GraphColoring *p2 = dynamic_cast<GraphColoring*>(prob2);

	return p1->solution.fitness < p2->solution.fitness;
}
