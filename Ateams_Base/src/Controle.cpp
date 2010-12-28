#include "Controle.h"

using namespace std;

pthread_mutex_t mutex;	// Mutex que protege a populacao principal
pthread_mutex_t mut_p;	// Mutex que protege as variaveis de criacao de novas solucoes
pthread_mutex_t mut_f;	// Mutex que protege a impressao das informacoes da execucao

sem_t semaphore;		// Semaforo que controla o acesso dos algoritmos ao processador

Controle::Controle()
{
	tamPop = 1000;
	critUnicidade = 1;
	iterAteams = 250;
	tentAteams = 100;
	maxTempo = 3600;
	numThreads = 4;
	makespanBest = -1;

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = critUnicidade == 1 ? fncomp1 : fncomp2;
	pop = new set<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	algs = new vector<Heuristica*>;

	actAlgs = new list<string>;
	actThreads = 0;

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mut_p, NULL);
	pthread_mutex_init(&mut_f, NULL);

	sem_init(&semaphore, 0, numThreads);
}

Controle::Controle(ParametrosATEAMS* pATEAMS)
{
	tamPop = pATEAMS->tamPopAteams != -1 ? pATEAMS->tamPopAteams : 1000;
	critUnicidade = pATEAMS->critUnicidade != -1 ? pATEAMS->critUnicidade : 1;
	iterAteams = pATEAMS->iterAteams != -1 ? pATEAMS->iterAteams: 100;
	tentAteams = pATEAMS->tentAteams != -1 ? pATEAMS->tentAteams: 50;
	maxTempo = pATEAMS->maxTempoAteams != -1 ? pATEAMS->maxTempoAteams : INT_MAX;
	numThreads = pATEAMS->numThreads != -1 ? pATEAMS->numThreads : 4;
	makespanBest = pATEAMS->makespanBest != -1 ? pATEAMS->makespanBest : -1;

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = critUnicidade == 1 ? fncomp1 : fncomp2;
	pop = new set<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	algs = new vector<Heuristica*>;

	actAlgs = new list<string>;
	actThreads = 0;

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mut_p, NULL);
	pthread_mutex_init(&mut_f, NULL);

	sem_init(&semaphore, 0, numThreads);
}

Controle::~Controle()
{
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;

	for(iter = pop->begin(); iter != pop->end(); iter++)
		delete *iter;

	pop->clear();
	delete pop;

	vector<Heuristica*>::reverse_iterator it;

	cout << endl << endl << "Execuções: " << execThreads << endl << endl;
	for(it = algs->rbegin(); it != algs->rend(); it++)
		delete *it;

	algs->clear();
	delete algs;

	actAlgs->clear();
	delete actAlgs;

	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&mut_p);
	pthread_mutex_destroy(&mut_f);
}

double Controle::sumFitnessMaximize(set<Problema*, bool(*)(Problema*, Problema*)> *probs, int n)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::const_iterator iter;
	double sum = 0, i = 0;

	for(i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++)
		sum += (*iter)->getFitnessMaximize();

	return sum;
}

double Controle::sumFitnessMaximize(vector<Problema*> *probs, int n)
{
	vector<Problema*>::const_iterator iter;
	double sum = 0, i = 0;

	for(i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++)
		sum += (*iter)->getFitnessMaximize();

	return sum;
}

double Controle::sumFitnessMinimize(set<Problema*, bool(*)(Problema*, Problema*)> *probs, int n)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::const_iterator iter;
	double sum = 0, i = 0;

	for(i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++)
		sum += (*iter)->getFitnessMinimize();

	return sum;
}

double Controle::sumFitnessMinimize(vector<Problema*> *probs, int n)
{
	vector<Problema*>::const_iterator iter;
	double sum = 0, i = 0;

	for(i = 0, iter = probs->begin(); i < n && iter != probs->end(); i++, iter++)
		sum += (*iter)->getFitnessMinimize();

	return sum;
}

set<Problema*, bool(*)(Problema*, Problema*)>::iterator Controle::selectRouletteWheel(set<Problema*, bool(*)(Problema*, Problema*)>* probs, double fitTotal, unsigned int randWheel)
{
	// Armazena o fitness total da populacao
	unsigned int sum = (unsigned int)fitTotal;
	// Um numero entre zero e "sum" e sorteado
	randWheel = xRand(randWheel, 0, sum+1);

	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	for(iter = probs->begin(); iter != probs->end(); iter++)
	{
		sum -= (int)(*iter)->getFitnessMaximize();
		if(sum <= randWheel)
		{
			return iter;
		}
	}
	return (probs->begin());
}

vector<Problema*>::iterator Controle::selectRouletteWheel(vector<Problema*>* probs, double fitTotal, unsigned int randWheel)
{
	// Armazena o fitness total da populacao
	unsigned int sum = (unsigned int)fitTotal;
	// Um numero entre zero e "sum" e sorteado
	randWheel = xRand(randWheel, 0, sum+1);

	vector<Problema*>::iterator iter;
	for(iter = probs->begin(); iter != probs->end(); iter++)
	{
		sum -= (int)(*iter)->getFitnessMaximize();
		if(sum <= randWheel)
		{
			return iter;
		}
	}
	return probs->begin();
}

Heuristica* Controle::selectRouletteWheel(vector<Heuristica*>* heuristc, unsigned int probTotal, unsigned int randWheel)
{
	// Armazena o fitness total da populacao
	unsigned int sum = probTotal;
	// Um numero entre zero e "sum" e sorteado
	randWheel = xRand(randWheel, 0, sum+1);

	for(int i = 0; i < (int)heuristc->size(); i++)
	{
		sum -= heuristc->at(i)->prob;
		if(sum <= randWheel)
		{
			return heuristc->at(i);
		}
	}
	return heuristc->at(0);
}

vector<Problema*>::iterator Controle::selectRandom(vector<Problema*>* probs, int randWheel)
{
	randWheel = randWheel % probs->size();

	vector<Problema*>::iterator iter = probs->begin();
	for(int i = 0; iter != probs->end() && i < randWheel; iter++, i++);

	return iter;
}

list<Problema*>::iterator Controle::findSol(list<Problema*> *vect, Problema *p)
{
	list<Problema*>::iterator iter;

	for(iter = vect->begin(); iter != vect->end(); iter++)
		if(fnequal1((*iter), p))
			return iter;

	return iter;
}


void Controle::addHeuristic(Heuristica* alg)
{
	algs->push_back(alg);

	sort(algs->begin(), algs->end(), cmpAlg);
}

list<Problema*>* Controle::getPop()
{
	list<Problema*>* sol = new list<Problema*>();
	set<Problema*, bool(*)(Problema*, Problema*)>::const_iterator iter;

	for(iter = pop->begin(); iter != pop->end(); iter++)
		sol->push_back(*iter);

	return sol;
}

Problema* Controle::getSol(int n)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::const_iterator iter = pop->begin();

	for(int i = 0; i <= n && iter != pop->end(); iter++);

	return Problema::copySoluction(**(--iter));
}

void Controle::getInfo(execInfo *info)
{
	info->diffTime = difftime(time2, time1);
	info->numExecs = execThreads;

	info->worstFitness = Problema::worst;
	info->bestFitness = Problema::best;
	info->expSol = Problema::totalNumInst;
}

Problema* Controle::start(list<Problema*>* popInicial)
{
	time(&time1);

	pthread_t *threads = (pthread_t*)malloc(iterAteams * sizeof(pthread_t));
	pthread_t threadTime;
	void* temp = NULL;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	srand(unsigned(time(NULL)));

	geraPop(popInicial);

	if(pop->size() == 0)
	{
		cout << endl << "Nenuma Solução Inicial Encontrada!" << endl << endl;

		exit(1);
	}

	iterMelhora = 0;

	Problema::best = (*pop->begin())->getFitness();
	Problema::worst = (*pop->rbegin())->getFitness();

	cout << endl << "Pior Solução Inicial: " << Problema::worst << endl << endl;
	cout << "Melhor Solução Inicial: " << Problema::best << endl << endl << endl;

	pair<int, Controle*>* par = NULL;
	long int ins = 0;
	execThreads = 0;

	if(pthread_create(&threadTime, &attr, pthrTime, (void*)this) != 0)
	{
		cout << endl << endl << "Erro na criação da Thread! (pthrTime)" << endl << endl;
		exit(1);
	}

	for(int execAteams = 0; execAteams < iterAteams; execAteams++)
	{
		par = new pair<int, Controle*>();
		par->first = execAteams+1;
		par->second = this;

		if(pthread_create(&threads[execAteams], &attr, pthrExec, (void*)par) != 0)
		{
			cout << endl << endl << "Erro na criação da Thread! (pthrExec)" << endl << endl;
			exit(1);
		}
	}

	for(int execAteams = 0; execAteams < iterAteams; execAteams++)
	{
		pthread_join(threads[execAteams], &temp);
		ins += (long int)temp;
	}

	PARAR = true;

	pthread_join(threadTime, NULL);

	cout << endl << "Soluções Permutadas: " << ins << endl;

	free(threads);

	pthread_attr_destroy(&attr);

	time(&time2);

	return Problema::copySoluction(**(pop->begin()));
}

inline int Controle::exec(int randomic, int eID)
{
	srand(randomic);

	Heuristica* alg = selectRouletteWheel(algs, Heuristica::numHeuristic, rand());
	vector<Problema*> *prob = NULL;
	pair<int, int>* ins;
	int contrib = 0;

	char cID[16];
	sprintf(cID, "%s(%.3d)", alg->name.c_str(), eID);
	string id = cID;

	string execNames;

	pthread_mutex_lock(&mut_f);
	{
		actThreads++;

		actAlgs->push_back(id);

		execNames = "";
		for(list<string>::iterator it = actAlgs->begin(); it != actAlgs->end(); it++)
			execNames = execNames + *it + " ";

		printf(">>> ALG: %s | ..................................................... | FILA: (%d : %s)\n", id.c_str(), actThreads, execNames.c_str());
	}
	pthread_mutex_unlock(&mut_f);

	prob = alg->start(pop, randomic);

	pthread_mutex_lock(&mutex);
	{
		double oldBest = Problema::best;

		ins = addSol(prob);
		execThreads++;

		double newBest = Problema::best;

		if(Problema::melhora(oldBest, newBest) > 0)
			iterMelhora = 0;
		else
			iterMelhora++;
	}
	pthread_mutex_unlock(&mutex);

	prob->clear();
	delete prob;

	pthread_mutex_lock(&mut_f);
	{
		actThreads--;

		actAlgs->erase(find(actAlgs->begin(), actAlgs->end(), id));

		printf("<<< ALG: %s | ITER: %.3d | FITNESS: %.6d:%.6d | CONTRIB: %.3d:%.3d", id.c_str(), execThreads, (int)Problema::best, (int)Problema::worst, ins->first, ins->second);

		execNames = "";
		for(list<string>::iterator it = actAlgs->begin(); it != actAlgs->end(); it++)
			execNames = execNames + *it + " ";

		printf(" | FILA: (%d : %s)\n", actThreads, execNames.c_str());
	}
	pthread_mutex_unlock(&mut_f);

	contrib = ins->second;

	delete ins;

	return contrib;
}

void* Controle::pthrExec(void *obj)
{
	pair<int, Controle*>* in = (pair<int, Controle*>*)obj;
	int execAteams = in->first;
	Controle *ctr = in->second;
	long int ins = 0;

	delete in;

	sem_wait(&semaphore);

	if(PARAR != true)
	{
		ins = ctr->exec(rand(), execAteams);

		if(ctr->iterMelhora > ctr->tentAteams || (ctr->makespanBest != -1 && Problema::melhora(ctr->makespanBest, Problema::best) >= 0))
			PARAR = true;
	}

	sem_post(&semaphore);

	return (void*)ins; 		//	pthread_exit((void*)ins);
}

void* Controle::pthrTime(void *obj)
{
	Controle *ctr = (Controle*)obj;
	time_t rawtime;

	while(PARAR == false)
	{
		time(&rawtime);

		if((int)difftime(rawtime, ctr->time1) > ctr->maxTempo)
			PARAR = true;

		sleep(1);
	}

	return NULL;			//	pthread_exit(NULL);
}

inline pair<int, int>* Controle::addSol(vector<Problema*> *news)
{
	pair<set<Problema*, bool(*)(Problema*, Problema*)>::iterator, bool> ret;
	vector<Problema*>::const_iterator iterNews;
	int nins = 0, nret = news->size();
	Problema* pointSol = NULL;

	for(iterNews = news->begin(); iterNews != news->end(); iterNews++)
	{
		if(Problema::melhora(**pop->rbegin(), **iterNews) < 0)
		{
			delete *iterNews;
		}
		else
		{
			ret = pop->insert(*iterNews);

			if(ret.second == true)
			{
				nins++;

				if((int)pop->size() > tamPop)
				{
					pointSol = *pop->rbegin();

					pop->erase(pointSol);
					delete pointSol;
				}
			}
			else
			{
				delete *iterNews;
			}
		}
	}

	Problema::best = (*pop->begin())->getFitness();
	Problema::worst = (*pop->rbegin())->getFitness();

	return new pair<int, int>(nret, nins);
}

inline void Controle::geraPop(list<Problema*>* popInicial)
{
	pair<set<Problema*, bool(*)(Problema*, Problema*)>::iterator, bool> ret;

	if(popInicial != NULL)
	{
		for(list<Problema*>::iterator iter = popInicial->begin(); iter != popInicial->end(); iter++)
		{
			if((int)pop->size() < tamPop)
			{
				ret = pop->insert(*iter);
				if(!ret.second)
					delete *iter;
			}
			else
			{
				delete *iter;
			}
		}
	}

	srand(unsigned(time(NULL)));

	unsigned long int limit = pow(tamPop, 3), iter = 0;
	Problema* soluction = NULL;

	cout << endl << "LOADING: " << flush;

	int loadingMax = 100, loading = ceil((int)pop->size()*loadingMax/tamPop);

	for(int i = 0; i < loading; i++)
		cout << '*' << flush;

	while((int)pop->size() < tamPop && iter < limit && PARAR == false)
	{
		soluction = Problema::randSoluction();

		if(soluction->getFitness() != -1)
		{
			ret = pop->insert(soluction);
			if(!ret.second)
			{
				iter++;

				delete soluction;
			}
			else
			{
				if((ceil((int)pop->size()*loadingMax/tamPop) - loading) == 1)
				{
					cout << '#' << flush;
					loading++;
				}

				if(iter > 0)
					iter--;
			}
		}
		else
		{
			iter++;

			delete soluction;
		}
	}

	cout << " " << loading << "%" << endl << endl;

	PARAR = false;

	return;
}

inline bool cmpAlg(Heuristica *h1, Heuristica *h2)
{
	return h1->prob < h2->prob;
}

int Heuristica::numHeuristic = 0;
