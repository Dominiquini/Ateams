#include "Controle.h"

using namespace std;

extern bool PARAR;

pthread_mutex_t mutex;
pthread_mutex_t mut_p;

Controle::Controle()
{
	tamPop = 1000;
	iterAteams = 100;
	numThreads = 4;
	maxTempo = INT_MAX;
	makespanBest = -1;

	omp_set_num_threads(numThreads);

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = fncomp1;
	pop = new set<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	algs = new vector<Heuristica*>;

	pthread_mutex_init(&mutex, NULL);
}

Controle::Controle(ParametrosATEAMS* pATEAMS)
{
	tamPop = pATEAMS->tamanhoPopulacao != -1 ? pATEAMS->tamanhoPopulacao : 1000;
	iterAteams = pATEAMS->iteracoesAteams != -1 ? pATEAMS->iteracoesAteams: 100;
	numThreads = pATEAMS->numThreads != -1 ? pATEAMS->numThreads : 4;
	maxTempo = pATEAMS->maxTempo != -1 ? pATEAMS->maxTempo : INT_MAX;
	makespanBest = pATEAMS->makespanBest != -1 ? pATEAMS->makespanBest : -1;

	omp_set_num_threads(numThreads);

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = fncomp1;
	pop = new set<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	algs = new vector<Heuristica*>;

	pthread_mutex_init(&mutex, NULL);
}

Controle::~Controle()
{
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;

	for(iter = pop->begin(); iter != pop->end(); iter++)
		delete *iter;

	pop->clear();
	delete pop;

	vector<Heuristica*>::iterator it;

	cout << endl << endl << "Execuções:" << endl << endl;
	for(it = algs->begin(); it != algs->end(); it++)
		delete *it;

	algs->clear();
	delete algs;

	pthread_mutex_destroy(&mutex);
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
	int threads_create = 0;
	int threads_join = 0;

	srand(unsigned(time(NULL)));

	geraPop();

	Problema::best = (*pop->begin())->getFitnessMinimize();
	Problema::worst = (*pop->rbegin())->getFitnessMinimize();

	cout << "(" << numThreads << " THREADS) CTR : " << Problema::best << " : " << Problema::worst << endl << endl << flush;

	struct timeval time1, time2;
	gettimeofday(&time1, NULL);

	int tempo = 0, ins = 0, execThreads = 0;

	string* algName;
	void *temp = NULL;
	vector<Problema*> *prob = NULL;
	pair<vector<Problema*>*, string*>* par = NULL;

	int cont = 0;
	while(true)
	{
		if(execThreads < numThreads && cont < iterAteams && PARAR == false)
		{
			pthread_create(&threads[threads_create++], NULL, run, (void*)this);
			execThreads++;
			cont++;
		}
		else if(execThreads > 0)
		{
			pthread_join(threads[threads_join++], &temp);
			execThreads--;

			par = (pair<vector<Problema*>*, string*>*)temp;

			prob = par->first;
			algName = par->second;

			pthread_mutex_lock(&mutex);
			ins = addSol(prob);
			pthread_mutex_unlock(&mutex);

			prob->clear();
			delete prob;

			Problema::best = (*pop->begin())->getFitnessMinimize();
			Problema::worst = (*pop->rbegin())->getFitnessMinimize();

			if(threads_join < 10)
				cout << *algName << " 00" << threads_join << " : " << Problema::best << " -> " << ins << endl << flush;
			else if(threads_join < 100)
				cout << *algName << " 0" << threads_join << " : " << Problema::best << " -> " << ins << endl << flush;
			else
				cout << *algName << " " << threads_join << " : " << Problema::best << " -> " << ins << endl << flush;


			if(Problema::best <= makespanBest)
				PARAR = true;

			if(tempo > maxTempo)
				PARAR = true;
		}
		else
		{
			break;
		}

		gettimeofday(&time2, NULL);
		tempo = time2.tv_sec - time1.tv_sec;
	}

	Problema::best = (*pop->begin())->getFitnessMinimize();
	Problema::worst = (*pop->rbegin())->getFitnessMinimize();

	free(threads);

	return *(pop->begin());
}

inline pair<vector<Problema*>*, string*>* Controle::exec(int randomic)
{
	Heuristica* alg = selectRouletteWheel(algs, Heuristica::numHeuristic, randomic);

	pair<vector<Problema*>*, string*>* par = new pair<vector<Problema*>*, string*>();

	par->first = alg->start(pop, randomic);
	par->second = new string(alg->name);

	return par;
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

void* Controle::run(void *obj)
{
	Controle *ctr = (Controle*)obj;
	void *ret = (void*)ctr->exec(rand());

	return ret;
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

inline bool cmpAlg(Heuristica *h1, Heuristica *h2)
{
	return h1->prob < h2->prob;
}

int Heuristica::numHeuristic = 0;
