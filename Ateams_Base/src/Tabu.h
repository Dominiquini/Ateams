#include "Problema.h"
#include "Heuristica.h"
#include "Controle.h"

using namespace std;

#ifndef _Tabu_
#define _Tabu_

class Tabu : public Heuristica
{
public:

	int iterTabu, tamListaTabu, tentSemMelhora, elitismo;
	float funcAsp, polExploracao;

	Tabu();
	Tabu(string, ParametrosHeuristicas&);
	virtual ~Tabu();

	vector<Problema*>* start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic, Heuristica_Listener* listener);

private:

	vector<Problema*>* exec(Problema*, Heuristica_Listener* listener);
};

bool isTabu(list<InfoTabu*> *listaTabu, InfoTabu *m);

void addTabu(list<InfoTabu*>* listaTabu, InfoTabu *m, int max);

bool aspiracao(double paramAsp, Problema *atual, Problema *local, Problema *global);

#endif
