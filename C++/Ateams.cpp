#include "Controle.h"

using namespace std;

int main()
{
	Controle ctr(100, (char*)"./dados/la01.prb");
	Problema* best = ctr.start();
	cout << endl << "Melhor Solução: " << best->makespan << endl << endl;

  return 0;
}
