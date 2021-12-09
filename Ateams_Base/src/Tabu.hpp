#include "Control.hpp"
#include "Heuristica.hpp"
#include "Problem.hpp"

using namespace std;

#ifndef _Tabu_
#define _Tabu_

class Tabu : public Heuristica
{
public:

	int iterTabu, tamListaTabu, tentSemMelhora, elitismo;
	float funcAsp, polExploracao;

	Tabu();
	~Tabu();

	bool setParameter(const char* parameter, const char* value);

	vector<Problem*>* start(set<Problem*, bool(*)(Problem*, Problem*)>* sol, Heuristica_Listener* listener);

private:

	vector<Problem*>* exec(Problem*, Heuristica_Listener* listener);
};

bool isTabu(list<InfoTabu*> *listaTabu, InfoTabu *m);

void addTabu(list<InfoTabu*>* listaTabu, InfoTabu *m, int max);

bool aspiracao(double paramAsp, Problem *atual, Problem *local, Problem *global);

#endif
