#include "Controle.h"

using namespace std;

pthread_mutex_t mutex;
pthread_mutex_t mut_p;

sem_t semaphore;

Controle::Controle()
{
	tamPop = 1000;
	iterAteams = 100;
	numThreads = 4;
	maxTempo = INT_MAX;
	makespanBest = -1;

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = fncomp1;
	pop = new set<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	algs = new vector<Heuristica*>;

	execAlgs = new list<string>;
	actThreads = 0;

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mut_p, NULL);

	sem_init(&semaphore, 0, numThreads);
}

Controle::Controle(ParametrosATEAMS* pATEAMS)
{
	tamPop = pATEAMS->tamPopAteams != -1 ? pATEAMS->tamPopAteams : 1000;
	iterAteams = pATEAMS->iterAteams != -1 ? pATEAMS->iterAteams: 100;
	numThreads = pATEAMS->numThreads != -1 ? pATEAMS->numThreads : 4;
	maxTempo = pATEAMS->maxTempoAteams != -1 ? pATEAMS->maxTempoAteams : INT_MAX;
	makespanBest = pATEAMS->makespanBest != -1 ? pATEAMS->makespanBest : -1;

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = fncomp1;
	pop = new set<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	algs = new vector<Heuristica*>;

	execAlgs = new list<string>;
	actThreads = 0;

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mut_p, NULL);

	sem_init(&semaphore, 0, numThreads);
}

Controle::~Controle()
{
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;

	for(iter = pop->begin(); iter != pop->end(); iter++)
		delete *iter;

	pop->clear();
	delete pop;

	vector<Heuristica*>::iterator it;

	cout << endl << endl << "Execuções: " << execThreads << endl << endl;
	for(it = algs->begin(); it != algs->end(); it++)
		delete *it;

	algs->clear();
	delete algs;

	execAlgs->clear();
	delete execAlgs;

	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&mut_p);
}

set<Problema*, bool(*)(Problema*, Problema*)>::iterator Controle::selectRouletteWheel(set<Problema*, bool(*)(Problema*, Problema*)>* pop, double fitTotal, unsigned int randWheel)
{
	// Armazena o fitness total da populacao
	unsigned int sum = fitTotal;
	// Um numero entre zero e "sum" e sorteado
	randWheel = xRand(randWheel, 0, sum+1);

	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	for(iter = pop->begin(); iter != pop->end(); iter++)
	{
		sum -= (int)(*iter)->getFitnessMaximize();
		if(sum <= randWheel)
		{
			return iter;
		}
	}
	return (pop->begin());
}

vector<Problema*>::iterator Controle::selectRouletteWheel(vector<Problema*>* pop, double fitTotal, unsigned int randWheel)
{
	// Armazena o fitness total da populacao
	unsigned int sum = fitTotal;
	// Um numero entre zero e "sum" e sorteado
	randWheel = xRand(randWheel, 0, sum+1);

	vector<Problema*>::iterator iter;
	for(iter = pop->begin(); iter != pop->end(); iter++)
	{
		sum -= (int)(*iter)->getFitnessMaximize();
		if(sum <= randWheel)
		{
			return iter;
		}
	}
	return pop->begin();
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

vector<Problema*>::iterator Controle::selectRandom(vector<Problema*>* pop, int randWheel)
{
	randWheel = randWheel % pop->size();

	vector<Problema*>::iterator iter = pop->begin();
	for(int i = 0; iter != pop->end() && i < randWheel; iter++, i++);

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
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;

	for(iter = pop->begin(); iter != pop->end(); iter++)
		sol->push_back(*iter);

	return sol;
}

Problema* Controle::getSol(int n)
{
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter = pop->begin();

	for(int i = 0; i <= n && iter != pop->end(); iter++);

	return *(--iter);
}

Problema* Controle::start()
{
	pthread_t *threads = (pthread_t*)malloc(iterAteams * sizeof(pthread_t));
	void* temp = NULL;

	srand(unsigned(time(NULL)));

	geraPop();

	Problema::best = (*pop->begin())->getFitnessMinimize();
	Problema::worst = (*pop->rbegin())->getFitnessMinimize();

	cout << endl << "Pior Soulução: " << Problema::worst << endl << endl;
	cout << "Melhor Solução: " << Problema::best << endl << endl << endl;

	gettimeofday(&time1, NULL);

	pair<int, Controle*>* par = NULL;
	long int ins = 0;
	execThreads = 0;

	for(int execAteams = 0; execAteams < iterAteams; execAteams++)
	{
		par = new pair<int, Controle*>();
		par->first = execAteams+1;
		par->second = this;

		pthread_create(&threads[execAteams], NULL, run, (void*)par);
	}

	for(int execAteams = 0; execAteams < iterAteams; execAteams++)
	{
		pthread_join(threads[execAteams], &temp);
		ins += (long int)temp;
	}

	cout << endl << "Soluções Permutadas: " << ins << endl;

	free(threads);

	return *(pop->begin());
}

inline pair<vector<Problema*>*, string*>* Controle::exec(int randomic, int eID)
{
	Heuristica* alg = selectRouletteWheel(algs, Heuristica::numHeuristic, randomic);

	pair<vector<Problema*>*, string*>* par = new pair<vector<Problema*>*, string*>();

	pthread_mutex_lock(&mutex);
	char cID[16];
	sprintf(cID, "%s(%.3d)", alg->name.c_str(), eID);
	string id = cID;

	actThreads++;

	execAlgs->push_back(id);
	pthread_mutex_unlock(&mutex);

	par->first = alg->start(pop, randomic);
	par->second = new string(alg->name);

	pthread_mutex_lock(&mutex);
	actThreads--;

	execAlgs->erase(find(execAlgs->begin(), execAlgs->end(), id));
	pthread_mutex_unlock(&mutex);

	return par;
}

void* Controle::run(void *obj)
{
	string *algName, execNames;
	vector<Problema*> *prob = NULL;
	pair<vector<Problema*>*, string*>* out = NULL;

	pair<int, Controle*>* in = (pair<int, Controle*>*)obj;
	int execAteams = in->first;
	Controle *ctr = in->second;

	long int ins = 0;

	sem_wait(&semaphore);

	if(PARAR != true)
	{
		out = ctr->exec(rand(), execAteams);

		prob = out->first;
		algName = out->second;

		pthread_mutex_lock(&mutex);

		ins = ctr->addSol(prob);

		prob->clear();
		delete prob;

		delete in;
		delete out;

		ctr->execThreads++;

		for(list<string>::iterator it = ctr->execAlgs->begin(); it != ctr->execAlgs->end(); it++)
			execNames = execNames + *it + " ";

		printf("ALG: %s(%.3d) | ITER: %.3d | MAKESPAN: %.4d | CONTRIB:  %.3ld", algName->c_str(),  execAteams, ctr->execThreads, Problema::best, ins);
		printf(" | FILA: (%d : %s)\n", ctr->actThreads, execNames.c_str());

		pthread_mutex_unlock(&mutex);

		struct timeval time2;
		gettimeofday(&time2, NULL);

		if(((time2.tv_sec - ctr->time1.tv_sec) > ctr->maxTempo) || (Problema::best <= ctr->makespanBest))
			PARAR = true;
	}

	sem_post(&semaphore);

	pthread_exit((void*)ins);
}

inline int Controle::addSol(vector<Problema*> *news)
{
	pair<set<Problema*, bool(*)(Problema*, Problema*)>::iterator, bool> ret;
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iterSol;
	vector<Problema*>::iterator iterNews;
	int ins = 0;

	for(iterNews = news->begin(); iterNews != news->end(); iterNews++)
	{
		ret = pop->insert(*iterNews);

		if(ret.second == true)
		{
			iterSol = pop->end();
			iterSol--;

			if((int)pop->size() > tamPop)
			{
				if(fnequal1(*iterNews, *iterSol))
					ins--;

				pop->erase(iterSol);
				delete *iterSol;
			}

			ins++;
		}
		else
		{
			delete *iterNews;
		}
	}
	Problema::best = (*pop->begin())->getFitnessMinimize();
	Problema::worst = (*pop->rbegin())->getFitnessMinimize();

	return ins;
}

inline void Controle::geraPop()
{
	srand(unsigned(time(NULL)));

	Problema* prob = NULL;
	pair<set<Problema*, bool(*)(Problema*, Problema*)>::iterator, bool> ret;
	while((int)pop->size() < tamPop)
	{
		prob = Problema::alloc();

		if(prob->getFitnessMinimize() != -1)
		{
			ret = pop->insert(prob);
			if(!ret.second)
				delete prob;
		}
		else
			delete prob;
	}
}

inline bool cmpAlg(Heuristica *h1, Heuristica *h2)
{
	return h1->prob < h2->prob;
}

int Heuristica::numHeuristic = 0;
