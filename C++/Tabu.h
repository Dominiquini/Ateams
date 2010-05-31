#include "Problema.h"
#include "Heuristica.h"
#include "Controle.h"

using namespace std;

#ifndef _Tabu_
#define _Tabu_

class Tabu : public Heuristica
{
public:

	int iterTabu, tamListaTabu, tentSemMelhora;
	float funcAsp, polExploracao;

	Tabu();
	Tabu(string, ParametrosBT*);
	~Tabu();

	vector<Problema*>* start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, int randomic);

private:

	vector<Problema*>* exec(Problema*);
};

bool isTabu(list<movTabu*> *listaTabu, movTabu *m);

void addTabu(list<movTabu*>* listaTabu, movTabu *m, int max);

double aspiracao(double paramAsp, double local, double global);

#endif
