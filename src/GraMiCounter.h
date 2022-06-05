#ifndef GRAMICOUNTER_H_
#define GRAMICOUNTER_H_

#include <tr1/unordered_set>
#include <vector>
#include "GraphX.h"
#include "Pattern.h"

class pair_with_edge
{
public:
	int id1;
	int id2;
	double edgeLabel;
	int minDomainSize;
};

class GraMiCounter
{
private:
	static void setDomainsOrder(tr1::unordered_map<int, tr1::unordered_set<int>*>& domains_values, int* order, Pattern* pattern);
public:
	static int numByPassedNodes;
	static bool useAC3;
	static int numSideEffectNodes;
	static int numPostponedNodes;

	static int isFrequent(GraphX*, Pattern*, int support, double approximate, map<int, set<int>> &domains_solutions);
	static int isFrequent_adv(GraphX*, Pattern*, int support, int subTaskNum, int subTaskMax, int* mniTable, tr1::unordered_map<int, tr1::unordered_set<int>* >* postponedNodes = 0, unsigned long maxIters = Settings::postponeNodesAfterIterations);
	static int isFrequent_approx(GraphX*, Pattern*, int support, int& invalidCol, int& predictedValids, bool allowEarlyBreak, bool& exact, unsigned long& numVisiteNodes, unsigned long& numIterations, vector<unsigned long>& listOfNumOfIters, unsigned long& predictedTime);
	static void AC_3(GraphX*, tr1::unordered_map<int, tr1::unordered_set<int>*>& , Pattern* , int support);
	static void AC_3(GraphX*, tr1::unordered_map<int, tr1::unordered_set<int>*>& , GraphX* , int support, int invalidCol = -1);
	static bool refine(GraphX*, tr1::unordered_set<int>* , tr1::unordered_set<int>* , pair_with_edge* , int );
	static bool isItAcyclic(GraphX& );

};

#endif /* GRAMICOUNTER_H_ */
