#include "Control.hpp"
#include "Heuristic.hpp"
#include "Problem.hpp"

using namespace std;

#ifndef _TABU_
#define _TABU_

class TabuSearch: public Heuristic {
public:

	int iterTabu, tamListaTabu, tentSemMelhora, elitism;
	float funcAsp, polExploracao;

	TabuSearch();
	~TabuSearch();

	bool setParameter(const char *parameter, const char *value) override;

	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) override;

	vector<Problem*>* start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicListener *listener) override;

	void markSolutions(vector<Problem*>* solutions) override;

private:

	vector<Problem*>* exec(Problem*, HeuristicListener *listener);
};

class InfoTabu {
public:

	virtual bool operator ==(InfoTabu&) = 0;

	InfoTabu() {
	}
	virtual ~InfoTabu() {
	}
};

bool isTabu(list<InfoTabu*> *listaTabu, InfoTabu *m);

void addTabu(list<InfoTabu*> *listaTabu, InfoTabu *m, int max);

bool aspiracao(double paramAsp, Problem *atual, Problem *local, Problem *global);

#endif
