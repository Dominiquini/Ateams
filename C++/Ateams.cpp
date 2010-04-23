#include "Controle.h"

using namespace std;

int main()
{
	Problema::leProblema(fopen("./dados/la01.prb", "r"));

	Controle* ctr = new Controle(250, 50, new Tabu(100, 25));
	Problema* best = ctr->start();
	cout << endl << "Melhor Solução: " << best->makespan << endl << endl;

	delete ctr;
	cout << Problema::numInst << " : " << ctr->pop->size() << endl << endl;

  return 0;
}
