#include "Controle.h"

using namespace std;

extern bool PARAR;

pthread_mutex_t mutex;
pthread_mutex_t mut_p;

Controle::Controle()
{
	tamPop = 500;
	numAteams = 50;
	numThreads = 1;
	maxTempo = INT_MAX;
	makespanBest = -1;

	srand(unsigned(time(NULL)));

	bool(*fn_pt)(Problema*, Problema*) = fncomp1;
	pop = new set<Problema*, bool(*)(Problema*, Problema*)>(fn_pt);

	algs = new vector<Heuristica*>;

	pthread_mutex_init(&mutex, NULL);
}

Controle::Controle(ParametrosATEAMS* pATEAMS)
{
	tamPop = pATEAMS->tamanhoPopulacao;
	numAteams = pATEAMS->iteracoesAteams;
	numThreads = pATEAMS->numThreads;
	maxTempo = pATEAMS->maxTempo;
	makespanBest = pATEAMS->makespanBest;

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

#ifdef THREADS
Problema* Controle::start()
{
	pthread_t *threads = (pthread_t*)malloc(numAteams * sizeof(pthread_t));
	int threads_create = 0;
	int threads_join = 0;

	srand(unsigned(time(NULL)));

	geraPop();

	Problema::best = (*pop->begin())->getMakespan();
	Problema::worst = (*pop->rbegin())->getMakespan();

	cout << "CTR : 0 : " << (*pop->begin())->getMakespan() << endl << endl << flush;

	struct timeval time1, time2;
	gettimeofday(&time1, NULL);

	int tempo = 0, ins = 0, execThreads = 0;

	void *temp = NULL;
	vector<Problema*> *prob = NULL;

	int cont = 0;
	while(true)
	{
		if(execThreads < numThreads && cont < numAteams && PARAR == false)
		{
			pthread_create(&threads[threads_create++], NULL, run, (void*)this);
			execThreads++;
			cont++;
		}
		else if(execThreads > 0)
		{
			pthread_join(threads[threads_join++], &temp);
			execThreads--;

			prob = (vector<Problema*>*)temp;

			pthread_mutex_lock(&mutex);
			ins = addSol(prob);
			pthread_mutex_unlock(&mutex);

			prob->clear();
			delete prob;

			Problema::best = (*pop->begin())->getMakespan();
			Problema::worst = (*pop->rbegin())->getMakespan();

			if(threads_join < 10)
				cout << "Iteração 00" << threads_join << " : " << Problema::best << " -> " << ins << endl << flush;
			else if(threads_join < 100)
				cout << "Iteração 0" << threads_join << " : " << Problema::best << " -> " << ins << endl << flush;
			else
				cout << "Iteração " << threads_join << " : " << Problema::best << " -> " << ins << endl << flush;


			if((*pop->begin())->getMakespan() <= makespanBest)
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

	Problema::best = (*pop->begin())->getMakespan();
	Problema::worst = (*pop->rbegin())->getMakespan();

	free(threads);

	return *(pop->begin());
}
#else
Problema* Controle::start()
{
	srand(unsigned(time(NULL)));

	geraPop();

	Problema::best = (*pop->begin())->getMakespan();
	Problema::worst = (*pop->rbegin())->getMakespan();

	cout << "CTR : 0 : " << (*pop->begin())->getMakespan() << endl << endl << flush;

	struct timeval time1, time2;
	gettimeofday(&time1, NULL);

	int tempo = 0, ins;

	vector<Problema*> *prob = NULL;

	for(int i = 0; i < numAteams && tempo < maxTempo; i++)
	{
		prob = exec(rand());

		ins = addSol(prob);

		prob->clear();
		delete prob;

		Problema::best = (*pop->begin())->getMakespan();
		Problema::worst = (*pop->rbegin())->getMakespan();

		if((i+1) < 10)
			cout << atual << " : 0" << i+1 << " : " << Problema::best << " -> " << ins << endl << flush;
		else
			cout << atual << " : " << i+1 << " : " << Problema::best << " -> " << ins << endl << flush;

		if((*pop->begin())->getMakespan() <= makespanBest)
			break;

		gettimeofday(&time2, NULL);
		tempo = time2.tv_sec - time1.tv_sec;

		if(PARAR == 1)
			break;
	}
	Problema::best = (*pop->begin())->getMakespan();
	Problema::worst = (*pop->rbegin())->getMakespan();

	return *(pop->begin());
}
#endif

inline vector<Problema*>* Controle::exec(int randomic)
{
	Heuristica* alg = selectRouletteWheel(algs, Heuristica::numHeuristic, randomic);
	atual = alg->name;
	return alg->start(pop, randomic);
}

inline int Controle::addSol(vector<Problema*> *prob)
{
	pair<set<Problema*, bool(*)(Problema*, Problema*)>::iterator, bool> ret;
	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	int ins = 0;

	for(register int j = 0; j < (int)prob->size(); j++)
	{
		ret = pop->insert(prob->at(j));
		if(ret.second == true)
		{
			Problema::totalMakespan += prob->at(j)->getFitness();

			iter = pop->end();
			iter--;

			if((int)pop->size() > tamPop)
			{
				if(ret.first == iter)
					ins--;

				Problema::totalMakespan -= (*iter)->getFitness();
				pop->erase(iter);
				delete *iter;
			}

			ins++;
		}
		else
		{
			delete prob->at(j);
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

		if(prob->getMakespan() != -1)
		{
			ret = pop->insert(prob);
			if(ret.second == true)
				Problema::totalMakespan += prob->getFitness();
			else
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

set<Problema*, bool(*)(Problema*, Problema*)>::iterator Controle::selectRouletteWheel(set<Problema*, bool(*)(Problema*, Problema*)>* pop, int fitTotal, int randWheel)
{
	// Armazena o fitness total da populacao
	int sum = fitTotal;
	// Um numero entre zero e "sum" e sorteado
	randWheel = randWheel % (sum + 1);

	set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;
	for(iter = pop->begin(); iter != pop->end(); iter++)
	{
		sum -= (int)(*iter)->getFitness();
		if(sum <= randWheel)
		{
			return iter;
		}
	}
	return (pop->begin());
}

vector<Problema*>::iterator Controle::selectRouletteWheel(vector<Problema*>* pop, int fitTotal, int randWheel)
{
	// Armazena o fitness total da populacao
	int sum = fitTotal;
	// Um numero entre zero e "sum" e sorteado
	randWheel = randWheel % (sum + 1);

	vector<Problema*>::iterator iter;
	for(iter = pop->begin(); iter != pop->end(); iter++)
	{
		sum -= (int)(*iter)->getFitness();
		if(sum <= randWheel)
		{
			return iter;
		}
	}
	return pop->begin();
}

Heuristica* Controle::selectRouletteWheel(vector<Heuristica*>* heuristc, int probTotal, int randWheel)
{
	// Armazena o fitness total da populacao
	int sum = probTotal;
	// Um numero entre zero e "sum" e sorteado
	randWheel = randWheel % (sum + 1);

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

inline bool cmpAlg(Heuristica *h1, Heuristica *h2)
{
	return h1->prob < h2->prob;
}

int Heuristica::numHeuristic = 0;
