#include "Control.hpp"
#include "Heuristic.hpp"
#include "Problem.hpp"

using namespace std;

#ifndef _Tabu_
#define _Tabu_

class TabuSearch : public Heuristic
{
public:

	int iterTabu, tamListaTabu, tentSemMelhora, elitism;
	float funcAsp, polExploracao;

	TabuSearch();
	~TabuSearch();

	bool setParameter(const char* parameter, const char* value);

	vector<Problem*>* start(set<Problem*, bool(*)(Problem*, Problem*)>* sol, HeuristicListener* listener);

private:

	vector<Problem*>* exec(Problem*, HeuristicListener* listener);
};

bool isTabu(list<InfoTabu*> *listaTabu, InfoTabu *m);

void addTabu(list<InfoTabu*>* listaTabu, InfoTabu *m, int max);

bool aspiracao(double paramAsp, Problem *atual, Problem *local, Problem *global);

#endif
