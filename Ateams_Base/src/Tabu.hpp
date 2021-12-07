#include "Problema.hpp"
#include "Heuristica.hpp"
#include "Controle.hpp"

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

	vector<Problema*>* start(set<Problema*, bool(*)(Problema*, Problema*)>* sol, Heuristica_Listener* listener);

private:

	vector<Problema*>* exec(Problema*, Heuristica_Listener* listener);
};

bool isTabu(list<InfoTabu*> *listaTabu, InfoTabu *m);

void addTabu(list<InfoTabu*>* listaTabu, InfoTabu *m, int max);

bool aspiracao(double paramAsp, Problema *atual, Problema *local, Problema *global);

#endif
