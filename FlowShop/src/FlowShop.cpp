#include "FlowShop.hpp"

using namespace std;

/* Static Members */

ProblemType Problem::TYPE = MINIMIZATION;

double Problem::best = 0;
double Problem::worst = 0;

unsigned int Problem::numInst = 0;
unsigned long long Problem::totalNumInst = 0;

unsigned int Problem::neighbors = 0;

char FlowShop::name[128];
int **FlowShop::time = NULL;
int FlowShop::njob = 0, FlowShop::nmaq = 0;

Problem* Problem::randomSolution() {
	return new FlowShop();
}

Problem* Problem::copySolution(const Problem &prob) {
	return new FlowShop(prob);
}

void Problem::readProblemFromFile(char *input) {
	FILE *f = fopen(input, "r");

	if (f == NULL)
		throw "Wrong Data File!";

	if (!fgets(FlowShop::name, 128, f))
		throw "Wrong Data File!";

	if (!fscanf(f, "%d %d", &FlowShop::njob, &FlowShop::nmaq))
		throw "Wrong Data File!";

	Problem::allocateMemory();

	for (int i = 0; i < FlowShop::njob; i++) {
		for (int j = 0; j < FlowShop::nmaq; j++) {
			if (!fscanf(f, "%*d %d", &FlowShop::time[i][j]))
				throw "Wrong Data File!";
		}
	}

	for (int i = 1; i < FlowShop::njob; i++)
		Problem::neighbors += i;

	fclose(f);

	return;
}

list<Problem*>* Problem::readPopulationFromLog(char *log) {
	FILE *f = fopen(log, "r");

	if (f != NULL) {
		list<Problem*> *popInicial = new list<Problem*>();
		int njob, nmaq, nprob, makespan;
		short int *prob;
		FlowShop *fs;

		if (!fscanf(f, "%d %d %d", &nprob, &nmaq, &njob))
			throw "Wrong Log File!";

		if (nmaq != FlowShop::nmaq || njob != FlowShop::njob)
			throw "Wrong Log File!";

		for (int i = 0; i < nprob; i++) {
			prob = (short int*) allocateMatrix<short int>(1, njob, 1, 1);

			if (!fscanf(f, "%d", &makespan))
				throw "Wrong Log File!";

			for (int j = 0; j < njob; j++) {
				if (!fscanf(f, "%hd", &prob[j]))
					throw "Wrong Log File!";
			}

			fs = new FlowShop(prob);

			if (makespan != fs->getFitness())
				throw "Wrong Log File!";

			popInicial->push_back(fs);
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
		short int *prob;

		fprintf(f, "%d %d %d\n\n", sizePop, FlowShop::nmaq, FlowShop::njob);

		for (iter = popInicial->begin(); iter != popInicial->end(); iter++) {
			FlowShop *fs = dynamic_cast<FlowShop*>(*iter);

			prob = fs->getSoluction().scaling;

			fprintf(f, "%d\n", (int) fs->getSoluction().fitness);

			for (int j = 0; j < FlowShop::njob; j++) {
				fprintf(f, "%.2d ", prob[j]);
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

	FlowShop::time = (int**) malloc(FlowShop::njob * sizeof(int*));

	for (int i = 0; i < FlowShop::njob; i++)
		FlowShop::time[i] = (int*) malloc(FlowShop::nmaq * sizeof(int));
}

void Problem::deallocateMemory() {
	Problem::best = 0;
	Problem::worst = 0;

	Problem::numInst = 0;
	Problem::totalNumInst = 0;

	Problem::neighbors = 0;

	for (int i = 0; i < FlowShop::njob; i++)
		free(FlowShop::time[i]);

	free(FlowShop::time);
}

/* Metodos */

FlowShop::FlowShop() : Problem::Problem() {
	solution.scaling = (short int*) allocateMatrix<short int>(1, njob, 1, 1);

	for (int j = 0; j < njob; j++) {
		solution.scaling[j] = j;
	}

	random_shuffle(&solution.scaling[0], &solution.scaling[njob], pointer_to_unary_function<int, int>(randomNumber));

	solution.staggering = NULL;

	calcFitness();
}

FlowShop::FlowShop(short int *prob) : Problem::Problem() {
	solution.scaling = prob;
	solution.staggering = NULL;

	calcFitness();
}

FlowShop::FlowShop(const Problem &prob) : Problem::Problem() {
	FlowShop *other = dynamic_cast<FlowShop*>(const_cast<Problem*>(&prob));

	this->solution.scaling = (short int*) allocateMatrix<short int>(1, njob, 1, 1);
	for (int j = 0; j < njob; j++)
		this->solution.scaling[j] = other->solution.scaling[j];

	this->solution.staggering = NULL;
	this->solution.fitness = other->solution.fitness;

	if (other->solution.staggering != NULL) {
		this->solution.staggering = (short int***) allocateMatrix<short int>(3, nmaq, njob, 3);
		for (int i = 0; i < nmaq; i++)
			for (int j = 0; j < njob; j++)
				for (int k = 0; k < 3; k++)
					this->solution.staggering[i][j][k] = other->solution.staggering[i][j][k];
	}
}

FlowShop::FlowShop(const Problem &prob, int pos1, int pos2) : Problem::Problem() {
	FlowShop *other = dynamic_cast<FlowShop*>(const_cast<Problem*>(&prob));

	this->solution.scaling = (short int*) allocateMatrix<short int>(1, njob, 1, 1);
	for (int j = 0; j < njob; j++)
		this->solution.scaling[j] = other->solution.scaling[j];

	short int aux = this->solution.scaling[pos1];
	this->solution.scaling[pos1] = this->solution.scaling[pos2];
	this->solution.scaling[pos2] = aux;

	this->solution.staggering = NULL;

	calcFitness();
}

FlowShop::~FlowShop() {
	deallocateMatrix<short int>(1, solution.scaling, 1, 1);

	deallocateMatrix<short int>(3, solution.staggering, FlowShop::nmaq, FlowShop::njob);
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
inline bool FlowShop::calcFitness() {
	deallocateMatrix<short int>(3, solution.staggering, nmaq, njob);

	short int ***aux_esc = (short int***) allocateMatrix<short int>(3, nmaq, njob, 3);
	short int *pos = (short int*) allocateMatrix<short int>(1, nmaq, 1, 1);
	int sum_time = 0;

	for (int m = 0; m < nmaq; m++)
		pos[m] = 0;

	for (int j = 0; j < njob; j++) {
		for (int m = 0; m < nmaq; m++) {
			aux_esc[m][j][0] = solution.scaling[j];

			if (m == 0) {
				aux_esc[m][j][1] = pos[m];
				aux_esc[m][j][2] = aux_esc[m][j][1] + time[solution.scaling[j]][m];
				pos[m] = aux_esc[m][j][2];
			} else {
				aux_esc[m][j][1] = aux_esc[m - 1][j][2] > pos[m] ? aux_esc[m - 1][j][2] : pos[m];
				aux_esc[m][j][2] = aux_esc[m][j][1] + time[solution.scaling[j]][m];
				pos[m] = aux_esc[m][j][2];
			}

			if (sum_time < aux_esc[m][j][2])
				sum_time = aux_esc[m][j][2];
		}
	}

	solution.staggering = aux_esc;
	solution.fitness = sum_time;

	deallocateMatrix<short int>(1, pos, 1, 1);

	return true;
}

inline void FlowShop::print(bool esc) {
	if (esc == true) {
		calcFitness();

		for (int i = 0; i < nmaq; i++) {
			printf("maq %d: ", i);
			for (int j = 0; j < njob; j++) {
				int k = solution.staggering[i][j][2] - solution.staggering[i][j][1];
				int spc = j == 0 ? solution.staggering[i][j][1] : solution.staggering[i][j][1] - solution.staggering[i][j - 1][2];

				while (spc--)
					printf(" ");

				while (k--)
					printf("%c", ((char) solution.staggering[i][j][0]) + 'a');
			}
			printf("\n");
		}

		printf("\n\nLabels:\n\n");

		for (int i = 0; i < njob; i++)
			printf("%c: job %d\n", ((char) i) + 'a', i);
	} else {
		for (int j = 0; j < njob; j++) {
			printf("%.2d ", solution.scaling[j]);
		}
		printf("\n");
	}
	return;
}

/* Retorna um vizinho aleatorio da solucao atual. */
inline Problem* FlowShop::neighbor() {
	int p1 = randomNumber(0, njob), p2 = randomNumber(0, njob);
	Problem *job = NULL;

	while (p2 == p1)
		p2 = randomNumber(0, njob);

	job = new FlowShop(*this, p1, p2);
	if (job->getFitness() != -1) {
		return job;
	} else {
		delete job;
		return NULL;
	}
}

/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual. */
inline vector<pair<Problem*, InfoTabu*>*>* FlowShop::localSearch() {
	if (Problem::neighbors > MAX_PERMUTATIONS)
		return localSearch((float) MAX_PERMUTATIONS / (float) Problem::neighbors);

	Problem *prob = NULL;
	int p1, p2;
	pair<Problem*, InfoTabu*> *temp;
	vector<pair<Problem*, InfoTabu*>*> *local = new vector<pair<Problem*, InfoTabu*>*>();

	for (p1 = 0; p1 < njob - 1; p1++) {
		for (p2 = p1 + 1; p2 < njob; p2++) {
			prob = new FlowShop(*this, p1, p2);

			if (prob->getFitness() != -1) {
				temp = new pair<Problem*, InfoTabu*>();
				temp->first = prob;
				temp->second = new InfoTabu_FlowShop(p1, p2);

				local->push_back(temp);
			} else {
				delete prob;
			}
		}
	}

	random_shuffle(local->begin(), local->end(), pointer_to_unary_function<int, int>(randomNumber));
	sort(local->begin(), local->end(), Problem::ptcomp);

	return local;
}

/* Retorna um conjunto de uma parcela das solucoes viaveis vizinhas da atual. */
inline vector<pair<Problem*, InfoTabu*>*>* FlowShop::localSearch(float parcela) {
	Problem *job = NULL;
	int p1, p2;
	pair<Problem*, InfoTabu*> *temp;
	vector<pair<Problem*, InfoTabu*>*> *local = new vector<pair<Problem*, InfoTabu*>*>();
	int def;

	def = (int) ((float) Problem::neighbors * parcela);

	if (def > MAX_PERMUTATIONS)
		def = MAX_PERMUTATIONS;

	for (int i = 0; i < def; i++) {
		p1 = randomNumber(0, njob), p2 = randomNumber(0, njob);

		while (p2 == p1)
			p2 = randomNumber(0, njob);

		job = new FlowShop(*this, p1, p2);
		if (job->getFitness() != -1) {
			temp = new pair<Problem*, InfoTabu*>();
			temp->first = job;
			temp->second = new InfoTabu_FlowShop(p1, p2);

			local->push_back(temp);
		} else {
			delete job;
		}
	}

	sort(local->begin(), local->end(), Problem::ptcomp);

	return local;
}

/* Realiza um crossover com uma outra solucao. Usa 2 pivos. */
inline pair<Problem*, Problem*>* FlowShop::crossOver(const Problem *parceiro, int partitionSize, int strength) {
	short int *f1 = (short int*) allocateMatrix<short int>(1, njob, 1, 1), *f2 = (short int*) allocateMatrix<short int>(1, njob, 1, 1);
	pair<Problem*, Problem*> *filhos = new pair<Problem*, Problem*>();
	int particao = partitionSize == 0 ? (FlowShop::njob) / 2 : partitionSize;
	int inicioPart = 0, fimPart = 0;

	FlowShop *other = dynamic_cast<FlowShop*>(const_cast<Problem*>(parceiro));

	inicioPart = randomNumber(0, njob);
	fimPart = inicioPart + particao <= njob ? inicioPart + particao : njob;

	swap_vect(this->solution.scaling, other->solution.scaling, f1, inicioPart, fimPart - inicioPart);
	swap_vect(other->solution.scaling, this->solution.scaling, f2, inicioPart, fimPart - inicioPart);

	filhos->first = new FlowShop(f1);
	filhos->second = new FlowShop(f2);

	return filhos;
}

/* Realiza um crossover com uma outra solucao. Usa 1 pivo. */
inline pair<Problem*, Problem*>* FlowShop::crossOver(const Problem *parceiro, int strength) {
	short int *f1 = (short int*) allocateMatrix<short int>(1, njob, 1, 1), *f2 = (short int*) allocateMatrix<short int>(1, njob, 1, 1);
	pair<Problem*, Problem*> *filhos = new pair<Problem*, Problem*>();
	int particao = 0;

	FlowShop *other = dynamic_cast<FlowShop*>(const_cast<Problem*>(parceiro));

	particao = randomNumber(1, njob);

	swap_vect(this->solution.scaling, other->solution.scaling, f1, 0, particao);
	swap_vect(other->solution.scaling, this->solution.scaling, f2, 0, particao);

	filhos->first = new FlowShop(f1);
	filhos->second = new FlowShop(f2);

	return filhos;
}

/* Devolve uma mutacao aleatoria na solucao atual. */
inline Problem* FlowShop::mutation(int mutMax) {
	short int *mut = (short int*) allocateMatrix<short int>(1, njob, 1, 1);
	Problem *vizinho = NULL, *temp = NULL, *mutacao = NULL;

	for (int j = 0; j < njob; j++)
		mut[j] = solution.scaling[j];

	temp = new FlowShop(mut);
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

inline double FlowShop::getFitness() const {
	return solution.fitness;
}

inline double FlowShop::getFitnessMaximize() const {
	return (double) INV_FITNESS / solution.fitness;
}

inline double FlowShop::getFitnessMinimize() const {
	return (double) solution.fitness;
}

/* Auxiliares */

inline void swap_vect(short int *p1, short int *p2, short int *f, int pos, int tam) {
	for (int i = pos; i < pos + tam; i++)
		f[i] = p1[i];

	for (int i = 0, j = 0; i < FlowShop::njob && j < FlowShop::njob; i++) {
		if (j == pos)
			j = pos + tam;

		if (find(&p1[pos], &p1[pos + tam], p2[i]) == &p1[pos + tam])
			f[j++] = p2[i];
	}
	return;
}

// comparator function:
bool fnEqualSolution(Problem *prob1, Problem *prob2) {
	FlowShop *p1 = dynamic_cast<FlowShop*>(prob1);
	FlowShop *p2 = dynamic_cast<FlowShop*>(prob2);

	if (p1->solution.fitness == p2->solution.fitness) {
		for (int j = 0; j < FlowShop::njob; j++)
			if (p1->solution.scaling[j] != p2->solution.scaling[j])
				return false;

		return true;
	} else
		return false;
}

// comparator function:
bool fnEqualFitness(Problem *prob1, Problem *prob2) {
	FlowShop *p1 = dynamic_cast<FlowShop*>(prob1);
	FlowShop *p2 = dynamic_cast<FlowShop*>(prob2);

	return p1->solution.fitness == p2->solution.fitness;
}

// comparator function:
bool fnSortSolution(Problem *prob1, Problem *prob2) {
	FlowShop *p1 = dynamic_cast<FlowShop*>(prob1);
	FlowShop *p2 = dynamic_cast<FlowShop*>(prob2);

	if (p1->solution.fitness == p2->solution.fitness) {
		for (int j = 0; j < FlowShop::njob; j++)
			if (p1->solution.scaling[j] != p2->solution.scaling[j])
				return p1->solution.scaling[j] < p2->solution.scaling[j];

		return false;
	} else
		return p1->solution.fitness < p2->solution.fitness;
}

// comparator function:
bool fnSortFitness(Problem *prob1, Problem *prob2) {
	FlowShop *p1 = dynamic_cast<FlowShop*>(prob1);
	FlowShop *p2 = dynamic_cast<FlowShop*>(prob2);

	return p1->solution.fitness < p2->solution.fitness;
}
