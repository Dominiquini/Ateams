#include "Controle.h"

using namespace std;

pthread_mutex_t mutex;	// Mutex que protege a populacao principal
pthread_mutex_t mut_p;	// Mutex que protege as variaveis de criacao de novas solucoes
pthread_mutex_t mut_f;	// Mutex que protege a impressao das informacoes da execucao

sem_t semaphore;		// Semaforo que controla o acesso dos algoritmos ao processador

Controle::Controle()
{
	tamPop = 1000;
	iterAteams = 100;
	tentAteams = 50;
	maxTempo = INT_MAX;
	numThreads = 4;
	makespanBest = -1;

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = fncomp1;
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
	iterAteams = pATEAMS->iterAteams != -1 ? pATEAMS->iterAteams: 100;
	tentAteams = pATEAMS->tentAteams != -1 ? pATEAMS->tentAteams: 50;
	maxTempo = pATEAMS->maxTempoAteams != -1 ? pATEAMS->maxTempoAteams : INT_MAX;
	numThreads = pATEAMS->numThreads != -1 ? pATEAMS->numThreads : 4;
	makespanBest = pATEAMS->makespanBest != -1 ? pATEAMS->makespanBest : -1;

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = fncomp1;
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

	vector<Heuristica*>::iterator it;

	cout << endl << endl << "Execuções: " << execThreads << endl << endl;
	for(it = algs->begin(); it != algs->end(); it++)
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
	unsigned int sum = fitTotal;
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
	unsigned int sum = fitTotal;
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

	return *(--iter);
}

Problema* Controle::start()
{
	pthread_t *threads = (pthread_t*)malloc(iterAteams * sizeof(pthread_t));
	void* temp = NULL;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setstacksize(&attr, (1024*1024*16));

	size_t size;
	pthread_attr_getstacksize(&attr, &size);

	srand(unsigned(time(NULL)));

	geraPop();

	iterMelhora = 0;

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

		pthread_create(&threads[execAteams], &attr, run, (void*)par);
	}

	for(int execAteams = 0; execAteams < iterAteams; execAteams++)
	{
		pthread_join(threads[execAteams], &temp);
		ins += (long int)temp;
	}

	cout << endl << "Soluções Permutadas: " << ins << endl;

	free(threads);

	pthread_attr_destroy(&attr);

	return *(pop->begin());
}

inline int Controle::exec(int randomic, int eID)
{
	srand(randomic);

	Heuristica* alg = selectRouletteWheel(algs, Heuristica::numHeuristic, rand());
	vector<Problema*> *prob = NULL;
	int ins = 0;

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

		printf(">>> ALG: %s) | .......................................... | FILA: (%d : %s)\n", id.c_str(), actThreads, execNames.c_str());
	}
	pthread_mutex_unlock(&mut_f);

	prob = alg->start(pop, randomic);

	pthread_mutex_lock(&mutex);
	{
		double oldBest = Problema::best;

		ins = addSol(prob);
		execThreads++;

		prob->clear();
		delete prob;

		double newBest = Problema::best;

		if(newBest < oldBest)
			iterMelhora = 0;
		else
			iterMelhora++;
	}
	pthread_mutex_unlock(&mutex);

	pthread_mutex_lock(&mut_f);
	{
		actThreads--;

		actAlgs->erase(find(actAlgs->begin(), actAlgs->end(), id));

		printf("<<< ALG: %s) | ITER: %.3d | MAKESPAN: %.4d | CONTRIB:  %.3d", id.c_str(), execThreads, Problema::best, ins);

		execNames = "";
		for(list<string>::iterator it = actAlgs->begin(); it != actAlgs->end(); it++)
			execNames = execNames + *it + " ";

		printf(" | FILA: (%d : %s)\n", actThreads, execNames.c_str());
	}
	pthread_mutex_unlock(&mut_f);

	return ins;
}

void* Controle::run(void *obj)
{
	pair<int, Controle*>* in = (pair<int, Controle*>*)obj;
	int execAteams = in->first;
	Controle *ctr = in->second;
	int ins = 0;

	sem_wait(&semaphore);

	if(PARAR != true)
	{
		ins = ctr->exec(rand(), execAteams);

		struct timeval time2;
		gettimeofday(&time2, NULL);

		if(((time2.tv_sec - ctr->time1.tv_sec) > ctr->maxTempo) || ctr->iterMelhora > ctr->tentAteams || (Problema::best <= ctr->makespanBest))
			PARAR = true;
	}

	sem_post(&semaphore);

	pthread_exit((void*)ins);
}

inline int Controle::addSol(vector<Problema*> *news)
{
	pair<set<Problema*, bool(*)(Problema*, Problema*)>::iterator, bool> ret;
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iterSol;
	vector<Problema*>::const_iterator iterNews;
	int ins = 0;

	for(iterNews = news->begin(); iterNews != news->end(); iterNews++)
	{
		if(**iterNews > **pop->rbegin())
		{
			delete *iterNews;
		}
		else
		{
			ret = pop->insert(*iterNews);

			if(ret.second == true)
			{
				ins++;

				if((int)pop->size() > tamPop)
				{
					iterSol = pop->end();
					iterSol--;

					pop->erase(iterSol);
					delete *iterSol;
				}
			}
			else
			{
				delete *iterNews;
			}
		}
	}

	Problema::best = (*pop->begin())->getFitnessMinimize();
	Problema::worst = (*pop->rbegin())->getFitnessMinimize();

	return ins;
}

inline void Controle::geraPop()
{
	srand(unsigned(time(NULL)));

	int t = 0;
	Problema* prob = NULL;
	pair<set<Problema*, bool(*)(Problema*, Problema*)>::iterator, bool> ret;
	while((int)pop->size() < tamPop && t < tamPop)
	{
		prob = Problema::alloc();

		if(prob->getFitnessMinimize() != -1)
		{
			t = 0;

			ret = pop->insert(prob);
			if(!ret.second)
				delete prob;
		}
		else
		{
			t++;

			delete prob;
		}
	}
}

inline bool cmpAlg(Heuristica *h1, Heuristica *h2)
{
	return h1->prob < h2->prob;
}

int Heuristica::numHeuristic = 0;
