#include <stdio.h>

#include <functional>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <set>

using namespace std;

#ifndef _PROBLEMA_
#define _PROBLEMA_

class Problema;

bool fncomp(Problema*, Problema*);

class Problema
{
public:
  static int numInst;

  static char name[128];
  static int **maq, **time;
  static int njob, nmaq;

  static void leProblema(FILE*);

  int **esc;
  int makespan;
  int ***escalon;

  Problema() {numInst++;}

  ~Problema() {numInst--;}

  virtual int calcMakespan() = 0;
  virtual void imprimir() = 0;

  // static set<Problema*, bool(*fn_pt)(Problema*, Problema*)> geraSolucao(int **maq, int **time);
  // static int escolheSolucao(set<Problema*, bool(*fn_pt)(Problema*, Problema*)> pop);

  // virtual int calcMakespan() = 0;
  // virtual set<Problema*, bool(*fn_pt)(Problema*, Problema*)> buscaLocal() = 0;
  // virtual Problema crossOver(Problema prob) = 0;
  // virtual Problema mutacao() = 0;
};

#endif
