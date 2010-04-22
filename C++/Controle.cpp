#include "JobShop.h"

using namespace std;

int main()
{
  srand(unsigned(time(NULL)));

  bool(*fn_pt)(Problema*, Problema*) = fncomp;
  set<Problema*, bool(*)(Problema*, Problema*)> sol(fn_pt);

  Problema::leProblema(fopen("./dados/la01.prb", "r"));

  while(sol.size() <= 100)
  {
	  Problema* prob = new JobShop();
	  if(prob->makespan != -1)
		  sol.insert(prob);
	  else
		  delete prob;
  }

  set<Problema*, bool(*)(Problema*, Problema*)>::iterator iter;

  for(iter = sol.begin(); iter != sol.end(); iter++)
	  cout << (*iter)->makespan << endl;

  return 0;
}
