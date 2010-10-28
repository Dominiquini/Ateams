#include "FlowShop.h"

using namespace std;

/* Static Members */

double Problema::best = 0;
double Problema::worst = 0;
int Problema::numInst = 0;
long int Problema::totalNumInst = 0;

char FlowShop::name[128];
short int **FlowShop::maq = NULL, **FlowShop::time = NULL;
int FlowShop::njob = 0, FlowShop::nmaq = 0;

int FlowShop::permutacoes = 0;

Problema* Problema::randSoluction()
{
	return new FlowShop();
}

Problema* Problema::copySoluction(const Problema& prob)
{
	return new FlowShop(prob);
}


void Problema::leProblema(FILE *f)
{
	if(!fgets (FlowShop::name, 128, f))
		exit(1);

	if(!fscanf (f, "%d %d", &FlowShop::njob, &FlowShop::nmaq))
		exit(1);

	FlowShop::maq = (short int**)alocaMatriz(2, FlowShop::njob, FlowShop::nmaq, 1);
	FlowShop::time = (short int**)alocaMatriz(2, FlowShop::njob, FlowShop::nmaq, 1);

	for (int i = 0; i < FlowShop::njob; i++)
	{
		for (int j = 0; j < FlowShop::nmaq; j++)
		{
			if (!fscanf (f, "%hd %hd", &FlowShop::maq[i][j], &FlowShop::time[i][j]))
				exit(1);
		}
	}

	for(int i = FlowShop::njob; i > 0; i--)
		FlowShop::permutacoes += i;
	FlowShop::permutacoes *= FlowShop::nmaq;

	return;
}

void Problema::leParametros(FILE *f, ParametrosATEAMS *pATEAMS, vector<ParametrosHeuristicas> *pHEURISTICAS)
{
	string parametros, ateams_p, alg_p;
	vector<size_t> algs;
	char temp = '#';
	int numAlgs = 0;
	float par = -1;
	size_t found = 0;

	while((temp = fgetc(f)) != EOF)
		parametros.push_back(temp);

	while((found = parametros.find((char*)"##("), found) != string::npos)
	{
		parametros[found] = '\0';
		algs.push_back(found+1);
	}
	algs.push_back(found);

	numAlgs = algs.size()-1;

	ateams_p.assign(parametros, 0, algs[0]);

	par = findPar(ateams_p, (char*)"[iterAteams]");
	pATEAMS->iterAteams = (int)par;

	par = findPar(ateams_p, (char*)"[numThreads]");
	pATEAMS->numThreads = (int)par;

	par = findPar(ateams_p, (char*)"[tentAteams]");
	pATEAMS->tentAteams = (int)par;

	par = findPar(ateams_p, (char*)"[maxTempoAteams]");
	pATEAMS->maxTempoAteams = (int)par;

	par = findPar(ateams_p, (char*)"[tamPopAteams]");
	pATEAMS->tamPopAteams = (int)par;

	par = findPar(ateams_p, (char*)"[critUnicidade]");
	pATEAMS->critUnicidade = (int)par;

	par = findPar(ateams_p, (char*)"[makespanBest]");
	pATEAMS->makespanBest = (int)par;


	ParametrosHeuristicas pTEMP;
	for(int i = 0; i < numAlgs; i++)
	{
		alg_p.assign(parametros, algs[i], algs[i+1]-algs[i]);
		alg_p[6] = '\0';

		if(alg_p.find((char*)"#(SA)#") != string::npos)
		{
			pTEMP.alg = SA;

			found = alg_p.find((char*)"(name) = ", 7) + strlen((char*)"(name) = ");
			sscanf(alg_p.c_str()+found, "%s", pTEMP.algName);

			alg_p += 7;

			par = findPar(alg_p, (char*)"[probSA]");
			pTEMP.probSA = (int)par;

			par = findPar(alg_p, (char*)"[polEscolhaSA]");
			pTEMP.polEscolhaSA = (int)par;

			par = findPar(alg_p, (char*)"[maxIterSA]");
			pTEMP.maxIterSA = (int)par;

			par = findPar(alg_p, (char*)"[initTempSA]");
			pTEMP.initTempSA = (float)par;

			par = findPar(alg_p, (char*)"[finalTempSA]");
			pTEMP.finalTempSA = (float)par;

			par = findPar(alg_p, (char*)"[restauraSolSA]");
			pTEMP.restauraSolSA = (int)par;

			par = findPar(alg_p, (char*)"[alphaSA]");
			pTEMP.alphaSA = (float)par;

			if(pTEMP.polEscolhaSA > pATEAMS->tamPopAteams)
				pTEMP.polEscolhaSA = pATEAMS->tamPopAteams;
		}
		else if(alg_p.find((char*)"#(AG)#") != string::npos)
		{
			pTEMP.alg = AG;

			found = alg_p.find((char*)"(name) = ", 7) + strlen((char*)"(name) = ");
			sscanf(alg_p.c_str()+found, "%s", pTEMP.algName);

			alg_p += 7;

			par = findPar(alg_p, (char*)"[probAG]");
			pTEMP.probAG = (int)par;

			par = findPar(alg_p, (char*)"[polEscolhaAG]");
			pTEMP.polEscolhaAG = (int)par;

			par = findPar(alg_p, (char*)"[iterAG]");
			pTEMP.iterAG = (int)par;

			par = findPar(alg_p, (char*)"[tamPopAG]");
			pTEMP.tamPopAG = (int)par;

			par = findPar(alg_p, (char*)"[tamParticaoAG]");
			pTEMP.tamParticaoAG = (int)par;

			par = findPar(alg_p, (char*)"[probCrossOverAG]");
			pTEMP.probCrossOverAG = (float)par;

			par = findPar(alg_p, (char*)"[probMutacaoAG]");
			pTEMP.probMutacaoAG = (float)par;

			if(pTEMP.polEscolhaAG > pATEAMS->tamPopAteams)
				pTEMP.polEscolhaAG = pATEAMS->tamPopAteams;
		}
		else if(alg_p.find((char*)"#(BT)#") != string::npos)
		{
			pTEMP.alg = BT;

			found = alg_p.find((char*)"(name) = ", 7) + strlen((char*)"(name) = ");
			sscanf(alg_p.c_str()+found, "%s", pTEMP.algName);

			alg_p += 7;

			par = findPar(alg_p, (char*)"[probBT]");
			pTEMP.probBT = (int)par;

			par = findPar(alg_p, (char*)"[polEscolhaBT]");
			pTEMP.polEscolhaBT = (int)par;

			par = findPar(alg_p, (char*)"[iterBT]");
			pTEMP.iterBT = (int)par;

			par = findPar(alg_p, (char*)"[tentSemMelhoraBT]");
			pTEMP.tentSemMelhoraBT = (int)par;

			par = findPar(alg_p, (char*)"[tamListaBT]");
			pTEMP.tamListaBT = (int)par;

			par = findPar(alg_p, (char*)"[polExplorBT]");
			pTEMP.polExplorBT = (float)par;

			par = findPar(alg_p, (char*)"[funcAspiracaoBT]");
			pTEMP.funcAspiracaoBT = (float)par;

			if(pTEMP.polEscolhaBT > pATEAMS->tamPopAteams)
				pTEMP.polEscolhaBT = pATEAMS->tamPopAteams;
		}
		else
		{
			printf("\n\nERRO NO ARQUIVO DE PARÂMETROS!!!\n\n");
			exit(0);
		}
		pHEURISTICAS->push_back(pTEMP);
	}
	return;
}

/* Le argumentos adicionais passados por linha de comando */
void Problema::leArgumentos(char **argv, int argc, ParametrosATEAMS *pATEAMS)
{
	int p = -1;

	if((p = findPosArgv(argv, argc, (char*)"--iterAteams")) != -1)
		pATEAMS->iterAteams = atoi(argv[p]);

	if((p = findPosArgv(argv, argc, (char*)"--numThreads")) != -1)
		pATEAMS->numThreads = atoi(argv[p]);

	if((p = findPosArgv(argv, argc, (char*)"--tentAteams")) != -1)
		pATEAMS->tentAteams = atoi(argv[p]);

	if((p = findPosArgv(argv, argc, (char*)"--maxTempoAteams")) != -1)
		pATEAMS->maxTempoAteams = atoi(argv[p]);

	if((p = findPosArgv(argv, argc, (char*)"--tamPopAteams")) != -1)
		pATEAMS->tamPopAteams = atoi(argv[p]);

	if((p = findPosArgv(argv, argc, (char*)"--critUnicidade")) != -1)
		pATEAMS->critUnicidade = atoi(argv[p]);

	if((p = findPosArgv(argv, argc, (char*)"--makespanBest")) != -1)
		pATEAMS->makespanBest = atoi(argv[p]);
}

list<Problema*>* Problema::lePopulacao(char *log)
{
	FILE *f = fopen(log, "r");

	if(f != NULL)
	{
		list<Problema*>* popInicial = new list<Problema*>();
		int njob, nmaq, nprob, makespan;
		short int *prob;
		Problema* p;

		if(!fscanf (f, "%d %d %d", &nprob, &nmaq, &njob))
		{
			printf("Arquivo de log incorreto!\n\n");
			exit(1);
		}

		if(nmaq != FlowShop::nmaq || njob != FlowShop::njob)
		{
			printf("Arquivo de log incorreto!\n\n");
			exit(1);
		}

		for(int i = 0; i < nprob; i++)
		{
			prob = (short int*)alocaMatriz(1, njob, 1, 1);

			if(!fscanf (f, "%d", &makespan))
			{
				printf("Arquivo de log incorreto!\n\n");
				exit(1);
			}

			for(int j = 0; j < njob; j++)
			{
				if(!fscanf (f, "%hd", &prob[j]))
				{
					printf("Arquivo de log incorreto!\n\n");
					exit(1);
				}
			}

			p = new FlowShop(prob);

			if(makespan != p->sol.makespan)
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
	short int *prob;

	fprintf(f, "%d %d %d\n\n", sizePop, FlowShop::nmaq, FlowShop::njob);

	for(iter = popInicial->begin(); iter != popInicial->end(); iter++)
	{
		prob = (*iter)->sol.esc;

		fprintf(f, "%d\n", (*iter)->sol.makespan);

		for(int j = 0; j < FlowShop::njob; j++)
		{
			fprintf(f, "%.2d ", prob[j]);
		}

		fprintf(f, "\n\n");
	}
	fclose(f);
}

void Problema::imprimeResultado(char *dados, char *parametros, execInfo *info, char *resultado)
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

void Problema::desalocaMemoria()
{
	desalocaMatriz(2, FlowShop::maq, FlowShop::njob, 1);
	desalocaMatriz(2, FlowShop::time, FlowShop::njob, 1);
}

double Problema::compare(double oldP, double newP)
{
	return oldP - newP;
}

double Problema::compare(Problema& oldP, Problema& newP)
{
	return oldP.getFitnessMinimize() - newP.getFitnessMinimize();
}

/* Metodos */

FlowShop::FlowShop() : Problema::Problema()
{
	sol.esc = (short int*)alocaMatriz(1, njob, 1, 1);

	for(int j = 0; j < njob; j++)
	{
		sol.esc[j] = j;
	}
	random_shuffle(&sol.esc[0], &sol.esc[njob]);

	sol.escalon = NULL;
	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}


FlowShop::FlowShop(short int *prob) : Problema::Problema()
{
	sol.esc = prob;

	sol.escalon = NULL;
	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

FlowShop::FlowShop(const Problema &prob) : Problema::Problema()
{
	sol.esc = (short int*)alocaMatriz(1, njob, 1, 1);
	for(int j = 0; j < njob; j++)
		sol.esc[j] = (prob.getSoluction()).esc[j];

	sol.escalon = NULL;
	sol.makespan = (prob.getSoluction()).makespan;

	if((prob.getSoluction()).escalon != NULL)
	{
		sol.escalon = (short int***)alocaMatriz(3, nmaq, njob, 3);
		for(int i = 0; i < nmaq; i++)
			for(int j = 0; j < njob; j++)
				for(int k = 0; k < 3; k++)
					sol.escalon[i][j][k] = (prob.getSoluction()).escalon[i][j][k];
	}
	exec = prob.exec;
}

FlowShop::FlowShop(const Problema &prob, int pos1, int pos2) : Problema::Problema()
{
	sol.esc = (short int*)alocaMatriz(1, njob, 1, 1);
	for(int j = 0; j < njob; j++)
		sol.esc[j] = (prob.getSoluction()).esc[j];

	short int aux = sol.esc[pos1];
	sol.esc[pos1] = sol.esc[pos2];
	sol.esc[pos2] = aux;

	sol.escalon = NULL;
	calcFitness(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

FlowShop::~FlowShop()
{
	if(sol.esc != NULL)
		desalocaMatriz(1, sol.esc, 1, 1);

	if(sol.escalon != NULL)
		desalocaMatriz(3, sol.escalon, FlowShop::nmaq, FlowShop::njob);
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
inline bool FlowShop::calcFitness(bool esc)
{
	short int ***aux_esc = (short int***)alocaMatriz(3, nmaq, njob, 3);
	short int *pos = (short int*)alocaMatriz(1, nmaq, 1, 1);
	int sum_time = 0;

	for(register int m = 0; m < nmaq; m++)
		pos[m] = 0;

	for(register int j = 0; j < njob; j++)
	{
		for(register int m = 0; m < nmaq; m++)
		{
			aux_esc[m][j][0] = sol.esc[j];

			if(m == 0)
			{
				aux_esc[m][j][1] = pos[m];
				aux_esc[m][j][2] = aux_esc[m][j][1] + time[sol.esc[j]][m];
				pos[m] = aux_esc[m][j][2];
			}
			else
			{
				aux_esc[m][j][1] = aux_esc[m - 1][j][2] > pos[m] ? aux_esc[m - 1][j][2] : pos[m];
				aux_esc[m][j][2] = aux_esc[m][j][1] + time[sol.esc[j]][m];
				pos[m] = aux_esc[m][j][2];
			}

			if(sum_time < aux_esc[m][j][2])
				sum_time = aux_esc[m][j][2];
		}
	}

	if(esc == false)
		desalocaMatriz(3, aux_esc, nmaq, njob);
	else
		sol.escalon = aux_esc;

	desalocaMatriz(1, pos, 1, 1);

	sol.makespan = sum_time;

	return true;
}

bool FlowShop::operator == (const Problema& p)
{
	return this->getFitnessMinimize() == p.getFitnessMinimize();
}

bool FlowShop::operator != (const Problema& p)
{
	return this->getFitnessMinimize() != p.getFitnessMinimize();
}

bool FlowShop::operator <= (const Problema& p)
{
	return this->getFitnessMinimize() <= p.getFitnessMinimize();
}

bool FlowShop::operator >= (const Problema& p)
{
	return this->getFitnessMinimize() >= p.getFitnessMinimize();
}

bool FlowShop::operator < (const Problema& p)
{
	return this->getFitnessMinimize() < p.getFitnessMinimize();
}

bool FlowShop::operator > (const Problema& p)
{
	return this->getFitnessMinimize() > p.getFitnessMinimize();
}

inline void FlowShop::imprimir(bool esc)
{
	if(esc == true)
	{
		calcFitness(esc);

		for(int i = 0; i < nmaq; i++)
		{
			printf("maq %d: ", i);
			for(int j = 0; j < njob; j++)
			{
				int k = sol.escalon[i][j][2] - sol.escalon[i][j][1];
				int spc = j == 0 ? sol.escalon[i][j][1] : sol.escalon[i][j][1] - sol.escalon[i][j-1][2];
				while(spc--)
					printf(" ");

				while(k--)
					printf("%c", ((char)sol.escalon[i][j][0]) + 'a');
			}
			printf("\n");
		}
		printf("\n\nLegenda:\n\n");

		for(int i = 0; i < njob; i++)
			printf("%c: job %d\n", ((char)i) + 'a', i);
	}
	else
	{
		for(int j = 0; j < njob; j++)
		{
			printf("%.2d ", sol.esc[j]);
		}
		printf("\n");
	}
	return;
}

/* Retorna um vizinho aleatorio da solucao atual. */
inline Problema* FlowShop::vizinho()
{
	int p1 = xRand(rand(), 0, njob), p2 = xRand(rand(), 0, njob);
	Problema *job = NULL;

	while(p2 == p1)
		p2 = xRand(rand(), 0, njob);

	job = new FlowShop(*this, p1, p2);
	if(job->getFitnessMinimize() != -1)
	{
		return job;
	}
	else
	{
		delete job;
		return NULL;
	}
}

/* Retorna um conjunto de todas as solucoes viaveis vizinhas da atual. */
inline vector<pair<Problema*, movTabu*>* >* FlowShop::buscaLocal()
{
	if(FlowShop::permutacoes > MAX_PERMUTACOES)
		return buscaLocal((float)MAX_PERMUTACOES/(float)FlowShop::permutacoes);

	Problema *job = NULL;
	register int p1, p2;
	pair<Problema*, movTabu*>* temp;
	vector<pair<Problema*, movTabu*>* >* local = new vector<pair<Problema*, movTabu*>* >();

	for(p1 = 0; p1 < njob-1; p1++)
	{
		for(p2 = p1+1; p2 < njob; p2++)
		{
			job = new FlowShop(*this, p1, p2);
			if(job->getFitnessMinimize() != -1)
			{
				temp = new pair<Problema*, movTabu*>();
				temp->first = job;
				temp->second = new movTabu(p1, p2);

				local->push_back(temp);
			}
			else
			{
				delete job;
			}
		}
	}

	random_shuffle(local->begin(), local->end());
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Retorna um conjunto de com uma parcela das solucoes viaveis vizinhas da atual. */
inline vector<pair<Problema*, movTabu*>* >* FlowShop::buscaLocal(float parcela)
{
	Problema *job = NULL;
	int p1, p2;
	pair<Problema*, movTabu*>* temp;
	vector<pair<Problema*, movTabu*>* >* local = new vector<pair<Problema*, movTabu*>* >();
	int def, i;

	def = (int)((float)FlowShop::permutacoes*parcela);

	if(def > MAX_PERMUTACOES)
		def = MAX_PERMUTACOES;

	for(i = 0; i < def; i++)
	{
		p1 = xRand(rand(), 0, njob), p2 = xRand(rand(), 0, njob);

		while(p2 == p1)
			p2 = xRand(rand(), 0, njob);

		job = new FlowShop(*this, p1, p2);
		if(job->getFitnessMinimize() != -1)
		{
			temp = new pair<Problema*, movTabu*>();
			temp->first = job;
			temp->second = new movTabu(p1, p2);

			local->push_back(temp);
		}
		else
		{
			delete job;
		}
	}
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Realiza um crossover com uma outra solucao. Usa 2 pivos. */
inline pair<Problema*, Problema*>* FlowShop::crossOver(const Problema* parceiro, int tamParticao)
{
	short int *f1 = (short int*)alocaMatriz(1, njob, 1, 1), *f2 = (short int*)alocaMatriz(1, njob, 1, 1);
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = tamParticao == -1 ? (FlowShop::njob)/2 : tamParticao;
	int inicioPart = 0, fimPart = 0;

	inicioPart = xRand(rand(), 0, njob);
	fimPart = inicioPart+particao <= njob ? inicioPart+particao : njob;

	swap_vect(this->sol.esc, (parceiro->getSoluction()).esc, f1, inicioPart, fimPart-inicioPart);
	swap_vect((parceiro->getSoluction()).esc, this->sol.esc, f2, inicioPart, fimPart-inicioPart);

	filhos->first = new FlowShop(f1);
	filhos->second = new FlowShop(f2);

	return filhos;
}

/* Realiza um crossover com uma outra solucao. Usa 1 pivo. */
inline pair<Problema*, Problema*>* FlowShop::crossOver(const Problema* parceiro)
{
	short int *f1 = (short int*)alocaMatriz(1, njob, 1, 1), *f2 = (short int*)alocaMatriz(1, njob, 1, 1);
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = 0;

	particao = xRand(rand(), 1, njob);

	swap_vect(this->sol.esc, (parceiro->getSoluction()).esc, f1, 0, particao);
	swap_vect((parceiro->getSoluction()).esc, this->sol.esc, f2, 0, particao);

	filhos->first = new FlowShop(f1);
	filhos->second = new FlowShop(f2);

	return filhos;
}

/* Devolve uma mutacao aleatoria na solucao atual. */
inline Problema* FlowShop::mutacao(int mutMax)
{
	short int *mut = (short int*)alocaMatriz(1, njob, 1, 1);
	Problema* vizinho = NULL, *temp = NULL, *mutacao = NULL;

	for(int j = 0; j < njob; j++)
		mut[j] = sol.esc[j];

	temp = new FlowShop(mut);
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

inline double FlowShop::getFitnessMaximize() const
{
	return (double)INV_FITNESS/sol.makespan;
}

inline double FlowShop::getFitnessMinimize() const
{
	return (double)sol.makespan;
}

/* Auxiliares */

inline void swap_vect(short int* p1, short int* p2, short int* f, int pos, int tam)
{
	for(int i = pos; i < pos+tam; i++)
		f[i] = p1[i];

	for(register int i = 0, j = 0; i < FlowShop::njob && j < FlowShop::njob; i++)
	{
		if(j == pos)
			j = pos+tam;

		if(find(&p1[pos], &p1[pos+tam], p2[i]) == &p1[pos+tam])
			f[j++] = p2[i];
	}
	return;
}


int findOrdem(int M, int maq, short int* job)
{
	for(int i = 0; i < M; i++)
		if(job[i] == maq)
			return i;
	return -1;
}

inline void* alocaMatriz(int dim, int a, int b, int c)
						{
	if(dim == 1)
	{
		short int *M = (short int*)malloc(a * sizeof(short int));

		return (void*)M;
	}
	else if(dim == 2)
	{
		short int **M = (short int**)malloc(a * sizeof (short int*));
		for (int i = 0; i < a; i++)
			M[i] = (short int*)malloc(b * sizeof (short int));

		return (void*)M;
	}
	else if(dim == 3)
	{
		short int ***M = (short int***)malloc(a * sizeof (short int**));
		for (int i = 0; i < a; i++)
		{
			M[i] = (short int**)malloc(b * sizeof(short int*));
			for(int j = 0; j < b; j++)
				M[i][j] = (short int*)malloc(c * sizeof (short int));
		}

		return (void*)M;
	}
	else
		return NULL;
						}

inline void desalocaMatriz(int dim, void *MMM, int a, int b)
{
	if(dim == 1)
	{
		short int *M = (short int*)MMM;

		free(M);
	}
	else if(dim == 2)
	{
		short int **M = (short int**)MMM;

		for(int i = 0; i < a; i++)
			free(M[i]);
		free(M);
	}
	else if(dim == 3)
	{
		short int ***M = (short int***)MMM;

		for(int i = 0; i < a; i++)
		{
			for(int j = 0; j < b; j++)
				free(M[i][j]);
			free(M[i]);
		}
		free(M);
	}
	return;
}

// comparator function:
bool fnequal1(Problema* p1, Problema* p2)
{
	if(p1->getFitnessMinimize() == p2->getFitnessMinimize())
	{
		for(int j = 0; j < FlowShop::njob; j++)
			if(p1->sol.esc[j] != p2->sol.esc[j])
				return false;

		return true;
	}
	else
		return false;
}

// comparator function:
bool fnequal2(Problema* p1, Problema* p2)
{
	return p1->getFitnessMinimize() == p2->getFitnessMinimize();
}

// comparator function:
bool fncomp1(Problema *prob1, Problema *prob2)
{
	if(prob1->getFitnessMinimize() == prob2->getFitnessMinimize())
	{
		for(int j = 0; j < FlowShop::njob; j++)
			if(prob1->sol.esc[j] != prob2->sol.esc[j])
				return prob1->sol.esc[j] < prob2->sol.esc[j];

		return false;
	}
	else
		return prob1->getFitnessMinimize() < prob2->getFitnessMinimize();
}

// comparator function:
bool fncomp2(Problema *prob1, Problema *prob2)
{
	return prob1->getFitnessMinimize() < prob2->getFitnessMinimize();
}


inline bool ptcomp(pair<Problema*, movTabu*>* p1, pair<Problema*, movTabu*>* p2)
{
	return (p1->first->getFitnessMinimize() > p2->first->getFitnessMinimize());
}
