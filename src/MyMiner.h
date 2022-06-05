/**
* 
MyMiner.h

 */

#ifndef MINERADV_H_
#define MINERADV_H_

#include "Miner.h"

class MyMiner : public Miner
{
public:
	static long maxNumCandids;
	map<int, set<int>> domains_solutions;
//	map<Pattern*, map<int, set<int>*>> frequentPatternsDomain;

	unsigned long numOfVisitedNodes;
	unsigned long numIterations;

	int nThreads;

	void startMining(string, int, int, int);
//	bool popACandidateApprox(vector<CLMap*>&, map<int, Pattern*>&, int destination);
//	void sendACandidateApprox(string, Pattern*, map<int, Pattern*>&, int);

	void setInputGraph(GraphX* g) { this->graph = g; }
	GraphX* getInputGraph() { return graph; }
	void printTotalExpectedTime();
    void printDomains();
    void printResult(tr1::unordered_set<int> delete_pattern_id);

    char*
    popMyCandidate(vector<CLMap *> &candidates, map<int, Pattern *> &currentlyChecking, int support,
                   double approximate);

    bool workCount(char* graphStr, int support, double approximate);

    int getFreq(Pattern *candidate, int support, double approximate);
};

#endif /* MINERADV_H_ */
