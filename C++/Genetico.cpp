#include "Controle.h"
#include "Genetico.h"

using namespace std;

extern int PARAR;

Genetico::Genetico()
{
	name = "DEFAULT_AG";
	prob = 50;
	polEscolha = -1;
	iterGenetico = 100;
	tamPopGenetico = 250;
	probCrossOver = 0.8;
	probMutacao = 0.1;
	tamParticionamento = -1;

	Heuristica::numHeuristic += prob;
}

Genetico::Genetico(ParametrosAG* pAG)
{
	name = "AG";
	prob = pAG->probAG;
	polEscolha = pAG->polEscolha;
	iterGenetico = pAG->numeroIteracoes;
	tamPopGenetico = pAG->tamanhoPopulacao;
	probCrossOver = pAG->probCrossOver;
	probMutacao = pAG->probMutacao;
	tamParticionamento = pAG->tamanhoParticionamento;

	Heuristica::numHeuristic += prob;
}

vector<Problema*>* Genetico::start(set<Problema*, bool(*)(Problema*, Problema*)>* sol)
{
	return new vector<Problema*>(1, new JobShop(**sol->begin()));
}
