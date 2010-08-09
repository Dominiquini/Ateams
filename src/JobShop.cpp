#include "JobShop.h"

using namespace std;

/* Static Members */

double Problema::best = 0;
double Problema::worst = 0;
int Problema::numInst = 0;
long int Problema::totalNumInst = 0;

char JobShop::name[128];
short int **JobShop::maq = NULL, **JobShop::time = NULL;
int JobShop::njob = 0, JobShop::nmaq = 0;

int JobShop::permutacoes = 0;

Problema* Problema::alloc()
{
	return new JobShop();
}

Problema* Problema::alloc(const Problema& prob)
{
	return new JobShop(prob);
}


void Problema::leProblema(FILE *f)
{
	if(!fgets (JobShop::name, 128, f))
		exit(1);

	if(!fscanf (f, "%d %d", &JobShop::njob, &JobShop::nmaq))
		exit(1);

	JobShop::maq = (short int**)alocaMatriz(2, JobShop::njob, JobShop::nmaq, 1);
	JobShop::time = (short int**)alocaMatriz(2, JobShop::njob, JobShop::nmaq, 1);

	for (int i = 0; i < JobShop::njob; i++)
	{
		for (int j = 0; j < JobShop::nmaq; j++)
		{
			if (!fscanf (f, "%hd %hd", &JobShop::maq[i][j], &JobShop::time[i][j]))
				exit(1);
		}
	}

	for(int i = JobShop::njob; i > 0; i--)
		JobShop::permutacoes += i;
	JobShop::permutacoes *= JobShop::nmaq;

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

	if((p = findPosArgv(argv, argc, (char*)"--makespanBest")) != -1)
		pATEAMS->makespanBest = atoi(argv[p]);
}

list<Problema*>* Problema::lePopulacao(FILE* f)
{
	if(f == NULL)
		return NULL;
	else
	{
		list<Problema*>* popInicial = new list<Problema*>();
		int njob, nmaq, nprob, makespan;
		short int **prob;
		Problema* p;

		if(!fscanf (f, "%d %d %d", &nprob, &nmaq, &njob))
			exit(1);

		for(int i = 0; i < nprob; i++)
		{
			prob = (short int**)alocaMatriz(2, nmaq, njob, 1);

			if(!fscanf (f, "%d", &makespan))
				exit(1);

			for(int i = 0; i < nmaq; i++)
			{
				for(int j = 0; j < njob; j++)
				{
					if(!fscanf (f, "%hd", &prob[i][j]))
						exit(1);
				}
			}
			p = new JobShop(prob);

			if(makespan != p->sol.makespan)
			{
				printf("Arquivo de log incorreto!\n\n");
				exit(1);
			}

			popInicial->push_back(p);
		}
		return popInicial;
	}
}

void Problema::escrevePopulacao(FILE* f, list<Problema*>* popInicial)
{
	if(f == NULL)
		return;

	int sizePop = (int)popInicial->size();
	list<Problema*>::iterator iter;
	short int **prob;

	fprintf(f, "%d %d %d\n\n", sizePop, JobShop::nmaq, JobShop::njob);

	for(iter = popInicial->begin(); iter != popInicial->end(); iter++)
	{
		prob = (*iter)->sol.esc;

		fprintf(f, "%d\n", (*iter)->sol.makespan);

		for(int j = 0; j < JobShop::nmaq; j++)
		{
			for(int m = 0; m < JobShop::njob; m++)
			{
				fprintf(f, "%.2d ", prob[j][m]);
			}
			fprintf(f, "\n");
		}
		fprintf(f, "\n");
	}
}

void Problema::imprimeResultado (struct timeval tv1, struct timeval tv2, FILE *resultados, int bestMakespan)
{
	int s = (((tv2.tv_sec*1000)+(tv2.tv_usec/1000)) - ((tv1.tv_sec*1000)+(tv1.tv_usec/1000)))/1000;

	fprintf(resultados, "%d\t%d\n", bestMakespan, s);
}

void Problema::desalocaMemoria()
{
	desalocaMatriz(2, JobShop::maq, JobShop::njob, 1);
	desalocaMatriz(2, JobShop::time, JobShop::njob, 1);
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

JobShop::JobShop() : Problema::Problema()
{
	short int *aux_vet, *aux_maq;

	sol.esc = (short int**)alocaMatriz(2, nmaq, njob, 1);
	aux_vet = (short int*)alocaMatriz(1, njob, 1, 1);
	aux_maq = (short int*)alocaMatriz(1, nmaq, 1, 1); 	// Indica proxima operacao da maquina

	for (int i = 0; i < nmaq; i++)
		aux_maq[i] = 0;

	for (int i = 0; i < njob; i++)
		aux_vet[i] = i;

	for (int i = 0; i < nmaq; i++)
	{
		random_shuffle(&aux_vet[0], &aux_vet[njob]);
		for (int j = 0; j < njob; j++)
		{
			sol.esc[maq[aux_vet[j]][i]][aux_maq[maq[aux_vet[j]][i]]] = aux_vet[j];
			aux_maq[maq[aux_vet[j]][i]] += 1;
		}
	}

	desalocaMatriz(1, aux_vet, 1, 1);
	desalocaMatriz(1, aux_maq, 1, 1);

	sol.escalon = NULL;
	calcMakespan(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}


JobShop::JobShop(short int **prob) : Problema::Problema()
{
	sol.esc = prob;

	sol.escalon = NULL;
	calcMakespan(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

JobShop::JobShop(const Problema &prob) : Problema::Problema()
{
	sol.esc = (short int**)alocaMatriz(2, nmaq, njob, 1);
	for(int i = 0; i < nmaq; i++)
		for(int j = 0; j < njob; j++)
			sol.esc[i][j] = (prob.getSoluction()).esc[i][j];

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

JobShop::JobShop(const Problema &prob, int maq, int pos1, int pos2) : Problema::Problema()
{
	sol.esc = (short int**)alocaMatriz(2, nmaq, njob, 1);
	for(int i = 0; i < nmaq; i++)
		for(int j = 0; j < njob; j++)
			sol.esc[i][j] = (prob.getSoluction()).esc[i][j];

	short int aux = sol.esc[maq][pos1];
	sol.esc[maq][pos1] = sol.esc[maq][pos2];
	sol.esc[maq][pos2] = aux;

	sol.escalon = NULL;
	calcMakespan(false);

	exec.tabu = false;
	exec.genetico = false;
	exec.annealing = false;
}

JobShop::~JobShop()
{
	if(sol.esc != NULL)
		desalocaMatriz(2, sol.esc, JobShop::nmaq, 0);

	if(sol.escalon != NULL)
		desalocaMatriz(3, sol.escalon, JobShop::nmaq, JobShop::njob);
}

/* Devolve o makespan  e o escalonamento quando a solucao for factivel, ou -1 quando for invalido. */
inline bool JobShop::calcMakespan(bool esc)
{
	register int max, cont = 0;
	short int ***aux_esc, **tmp, *pos;
	int ajob, apos, ainic, afim, sum_time;

	pos = (short int*)alocaMatriz(1, nmaq, 1, 1);
	tmp = (short int**)alocaMatriz(2, njob, nmaq+1, 1);
	aux_esc = (short int***)alocaMatriz(3, nmaq, njob, 3);

	register int i, j, k;

	for (i = 0; i < njob; i++)
		for(j = 0; j <= nmaq; j++)
			tmp[i][j] = j == 0 ? 0 : -1;

	for (i = 0; i < nmaq; i++)
		for(j = 0; j < njob; j++)
			aux_esc[i][j][0] = -1;

	for(i = 0; i < nmaq; i++)
		pos[i] = 0;

	max = nmaq * njob;

	i = 0, k = 0;
	while((cont < max) && k <= njob)
	{
		if(pos[i] != -1)
		{
			ajob = sol.esc[i][pos[i]];
			apos = findOrdem(nmaq, i, maq[ajob]);
			ainic = tmp[ajob][apos];
		}
		else
			ainic = -1;

		if(ainic != -1)
		{
			cont++;
			k = 0;

			aux_esc[i][pos[i]][0] = ajob;
			if(pos[i] == 0)
			{
				aux_esc[i][pos[i]][1] = ainic;
			}
			else
			{
				afim = aux_esc[i][pos[i] - 1][2];

				aux_esc[i][pos[i]][1] = afim > ainic ? afim : ainic;
			}
			aux_esc[i][pos[i]][2] = aux_esc[i][pos[i]][1] + time[ajob][apos];

			tmp[ajob][apos+1] = aux_esc[i][pos[i]][2];

			pos[i] = pos[i] == njob - 1 ? -1 : pos[i] + 1;
		}
		else
		{
			k++;
			i = (i + 1) % nmaq;
		}
	}

	if(k != njob + 1)
	{
		for(sum_time = 0, i = 0; i < njob; i++)
		{
			if(tmp[i][nmaq] > sum_time)
				sum_time = tmp[i][nmaq];
		}
		desalocaMatriz(2, tmp, njob, 0);
		desalocaMatriz(1, pos, 0, 0);

		if(esc == false)
			desalocaMatriz(3, aux_esc, nmaq, njob);
		else
			sol.escalon = aux_esc;

		sol.makespan = sum_time;
		return true;
	}
	else
	{
		desalocaMatriz(3, aux_esc, nmaq, njob);
		desalocaMatriz(2, tmp, njob, 0);
		desalocaMatriz(1, pos, 0, 0);

		sol.makespan = -1;
		return false;
	}
}

bool JobShop::operator == (Problema& p)
{
	return this->getFitnessMinimize() == p.getFitnessMinimize();
}

bool JobShop::operator != (Problema& p)
{
	return this->getFitnessMinimize() != p.getFitnessMinimize();
}

bool JobShop::operator <= (Problema& p)
{
	return this->getFitnessMinimize() <= p.getFitnessMinimize();
}

bool JobShop::operator >= (Problema& p)
{
	return this->getFitnessMinimize() >= p.getFitnessMinimize();
}

bool JobShop::operator < (Problema& p)
{
	return this->getFitnessMinimize() < p.getFitnessMinimize();
}

bool JobShop::operator > (Problema& p)
{
	return this->getFitnessMinimize() > p.getFitnessMinimize();
}

inline void JobShop::imprimir(bool esc)
{
	if(esc == true)
	{
		calcMakespan(esc);

		printf("\n");

		for(int i = 0; i < nmaq; i++)
		{
			printf("maq %d: ", i+1);
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
		printf("\nLegenda:\n\n");

		for(int i = 0; i < njob; i++)
			printf("%c: %d\n", ((char)i) + 'a', i);
	}
	else
	{
		for(int i = 0; i < nmaq; i++)
		{
			for(int j = 0; j < njob; j++)
			{
				printf("%.2d ", sol.esc[i][j]);
			}
			printf("\n");
		}
	}
	return;
}

/* Retorna um vizinho aleatorio da solucao atual. */
inline Problema* JobShop::vizinho()
{
	int maq = xRand(rand(), 0, nmaq), p1 = xRand(rand(), 0, njob), p2 = xRand(rand(), 0, njob);
	Problema *job = NULL;

	while(p2 == p1)
		p2 = xRand(rand(), 0, njob);

	job = new JobShop(*this, maq, p1, p2);
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
inline vector<pair<Problema*, movTabu*>* >* JobShop::buscaLocal()
{
	if(JobShop::permutacoes > MAX_PERMUTACOES)
		return buscaLocal((float)MAX_PERMUTACOES/(float)JobShop::permutacoes);

	Problema *job = NULL;
	register int maq, p1, p2;
	int numMaqs = nmaq, numJobs = njob;
	pair<Problema*, movTabu*>* temp;
	vector<pair<Problema*, movTabu*>* >* local = new vector<pair<Problema*, movTabu*>* >();

	for(maq = 0; maq < numMaqs; maq++)
	{
		for(p1 = 0; p1 < numJobs-1; p1++)
		{
			for(p2 = p1+1; p2 < numJobs; p2++)
			{
				job = new JobShop(*this, maq, p1, p2);
				if(job->getFitnessMinimize() != -1)
				{
					temp = new pair<Problema*, movTabu*>();
					temp->first = job;
					temp->second = new movTabu(maq, p1, p2);

					local->push_back(temp);
				}
				else
				{
					delete job;
				}
			}
		}
	}
	random_shuffle(local->begin(), local->end());
	sort(local->begin(), local->end(), ptcomp);

	return local;
}

/* Retorna um conjunto de com uma parcela das solucoes viaveis vizinhas da atual. */
inline vector<pair<Problema*, movTabu*>* >* JobShop::buscaLocal(float parcela)
{
	Problema *job = NULL;
	int maq, p1, p2;
	int numMaqs = nmaq, numJobs = njob;
	pair<Problema*, movTabu*>* temp;
	vector<pair<Problema*, movTabu*>* >* local = new vector<pair<Problema*, movTabu*>* >();
	int def, i;

	def = (int)((float)JobShop::permutacoes*parcela);

	if(def > MAX_PERMUTACOES)
		def = MAX_PERMUTACOES;

	for(i = 0; i < def; i++)
	{
		maq = xRand(rand(), 0, numMaqs), p1 = xRand(rand(), 0, numJobs), p2 = xRand(rand(), 0, numJobs);

		while(p2 == p1)
			p2 = xRand(rand(), 0, numJobs);

		job = new JobShop(*this, maq, p1, p2);
		if(job->getFitnessMinimize() != -1)
		{
			temp = new pair<Problema*, movTabu*>();
			temp->first = job;
			temp->second = new movTabu(maq, p1, p2);

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
inline pair<Problema*, Problema*>* JobShop::crossOver(Problema* parceiro, int tamParticao)
{
	short int **f1 = (short int**)alocaMatriz(2, nmaq, njob, 1), **f2 = (short int**)alocaMatriz(2, nmaq, njob, 1);
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = tamParticao == -1 ? (JobShop::njob)/2 : tamParticao;
	int inicioPart = 0, fimPart = 0;

	for(register int i = 0; i < nmaq; i++)
	{
		inicioPart = xRand(rand(), 0, njob);
		fimPart = inicioPart+particao <= njob ? inicioPart+particao : njob;

		swap_vect(this->sol.esc[i], (parceiro->getSoluction()).esc[i], f1[i], inicioPart, fimPart-inicioPart);
		swap_vect((parceiro->getSoluction()).esc[i], this->sol.esc[i], f2[i], inicioPart, fimPart-inicioPart);
	}

	filhos->first = new JobShop(f1);
	filhos->second = new JobShop(f2);

	return filhos;
}

/* Realiza um crossover com uma outra solucao. Usa 1 pivo. */
inline pair<Problema*, Problema*>* JobShop::crossOver(Problema* parceiro)
{
	short int **f1 = (short int**)alocaMatriz(2, nmaq, njob, 1), **f2 = (short int**)alocaMatriz(2, nmaq, njob, 1);
	pair<Problema*, Problema*>* filhos = new pair<Problema*, Problema*>();
	int particao = 0;

	for(register int i = 0; i < nmaq; i++)
	{
		particao = xRand(rand(), 1, njob);

		swap_vect(this->sol.esc[i], (parceiro->getSoluction()).esc[i], f1[i], 0, particao);
		swap_vect((parceiro->getSoluction()).esc[i], this->sol.esc[i], f2[i], 0, particao);
	}

	filhos->first = new JobShop(f1);
	filhos->second = new JobShop(f2);

	return filhos;
}

/* Devolve uma mutacao aleatoria na solucao atual. */
inline Problema* JobShop::mutacao(int mutMax)
{
	short int **mut = (short int**)alocaMatriz(2, nmaq, njob, 1);
	Problema* vizinho = NULL, *temp = NULL, *mutacao = NULL;

	for(int i = 0; i < nmaq; i++)
		for(int j = 0; j < njob; j++)
			mut[i][j] = sol.esc[i][j];

	temp = new JobShop(mut);
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

inline double JobShop::getFitnessMaximize()
{
	return (double)INV_FITNESS/sol.makespan;
}

inline double JobShop::getFitnessMinimize()
{
	return (double)sol.makespan;
}

/* Auxiliares */

inline void swap_vect(short int* p1, short int* p2, short int* f, int pos, int tam)
{
	for(int i = pos; i < pos+tam; i++)
		f[i] = p1[i];

	for(register int i = 0, j = 0; i < JobShop::njob && j < JobShop::njob; i++)
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
		for(int i = 0; i < JobShop::nmaq; i++)
			for(int j = 0; j < JobShop::njob; j++)
				if(p1->sol.esc[i][j] != p2->sol.esc[i][j])
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
		for(int i = 0; i < JobShop::nmaq; i++)
			for(int j = 0; j < JobShop::njob; j++)
				if(prob1->sol.esc[i][j] != prob2->sol.esc[i][j])
					return prob1->sol.esc[i][j] < prob2->sol.esc[i][j];
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
