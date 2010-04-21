#include "JobShop.h"

using namespace std;

int main()
{
  srand(unsigned(time(NULL)));

  bool(*fn_pt)(Problema*, Problema*) = fncomp;
  set<Problema*, bool(*)(Problema*, Problema*)> sol(fn_pt);

  Problema::leProblema(fopen("la01.prb", "r"));

  Problema* prob = new JobShop();

  return 0;
}
