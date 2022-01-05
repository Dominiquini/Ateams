#include "Control.hpp"
#include "Heuristic.hpp"
#include "Problem.hpp"

using namespace std;

#ifndef _TABU_
#define _TABU_

#define TABU_SEARCH_NAME "TabuSearch"

struct TabuSearchParameters : HeuristicParameters {
	map<string, void*> tabuSearchKeys = {
			{"probBT", &choiceProbability},
			{"choicePolicyTS", &choicePolicy},
			{"iterationsTS", &iterations},
			{"listSizeTS", &listSize},
			{"attemptsWithoutImprovementTS", &attemptsWithoutImprovement},
			{"elitismProbabilityTS", &elitismProbability},
			{"aspirationCriteriaTS", &aspirationCriteria},
			{"explorationPolicyTS", &explorationPolicy}
	};

	int iterations = 750;
	int listSize = 10;
	int	attemptsWithoutImprovement = 500;

	float elitismProbability = 0.25;

	float aspirationCriteria = 0.5;
	float explorationPolicy = 0.5;

	TabuSearchParameters() {
		keys.insert(tabuSearchKeys.begin(), tabuSearchKeys.end());
	}
};

class TabuSearch: public Heuristic {
public:

	TabuSearchParameters parameters;

	TabuSearch();
	~TabuSearch();

	HeuristicParameters getParameters() override;

	bool setParameter(const char *parameter, const char *value) override;

	set<Problem*, bool (*)(Problem*, Problem*)>::const_iterator selectRouletteWheel(set<Problem*, bool (*)(Problem*, Problem*)> *population, double fitTotal) override;

	vector<Problem*>* start(set<Problem*, bool (*)(Problem*, Problem*)> *sol, HeuristicExecutionInfo *listener) override;

	void markSolutions(vector<Problem*>* solutions) override;

private:

	vector<Problem*>* exec(Problem*, HeuristicExecutionInfo *listener);
};

class InfoTabu {
public:

	virtual bool operator ==(InfoTabu&) = 0;

	InfoTabu() {
	}
	virtual ~InfoTabu() {
	}
};

bool isTabu(list<InfoTabu*> *listaTabu, InfoTabu *m);

void addTabu(list<InfoTabu*> *listaTabu, InfoTabu *m, int max);

bool aspiracao(double paramAsp, Problem *atual, Problem *local, Problem *global);

#endif
