#include "Controle.h"

using namespace std;

int main()
{
	Controle* ctr = new Controle(250, (char*)"./dados/la01.prb");
	Problema* best = ctr->start();
	cout << endl << "Melhor Solução: " << best->makespan << endl << endl;

	cout << Problema::numInst << " : " << ctr->pop->size() << endl;
	delete ctr;
	cout << Problema::numInst << " : " << ctr->pop->size() << endl << endl;

  return 0;
}
