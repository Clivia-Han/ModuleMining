#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <limits>
#include <math.h>
#include <cstdlib>
#include<sys/time.h>
#include "GraMiCounter.h"
#include "utils.h"
#include "Settings.h"

int GraMiCounter::numByPassedNodes;
int GraMiCounter::numSideEffectNodes;
bool GraMiCounter::useAC3;

//utility functions
long long getmsofday_3()
{
   struct timeval tv;
   struct timezone tz;
   gettimeofday(&tv, &tz);
   return (long long)tv.tv_sec*1000 + tv.tv_usec/1000;
}

string getSig(int a, int b, double el)
{
	char ch[20];

	string sig;
	if(a<b)
		sig = intToString(a)+"_"+intToString(b)+"_"+doubleToString(el);
	else
		sig = intToString(b)+"_"+intToString(a)+"_"+doubleToString(el);
	return sig;
}

//function to delete results space
void deleteResults(vector<map<int, int>* >& result)
{
	for(auto & iter1 : result)
	{
		delete iter1;
	}
	result.clear();
}

/**
 * ModuleMining based approximate counter, for the 'pattern' in the 'graph' given the 'support' minimum threshold.
 * the 'approximate' parameter indicates whether we allow approximation or not, if approximate==-1 then it is not approximate
 */
// 这里解决给定子图，计算支持度的问题，缺少子图扩展和候选集生成步骤，已用上了isFrequentHeuristic中的改进
// 实现了unique label优化策略（isFrequentAdv中有）
int GraMiCounter::isFrequent(GraphX* graph, Pattern* pattern, int support, double approximate, map<int, set<int>>& domains_solutions)
{
	int freq = 0;

	//create domains
    // domain_values: pNodeID -> set(gNodeID)
	tr1::unordered_map<int, tr1::unordered_set<int>*> domains_values;
	GraphX* pg = pattern->getGraph();
	for(tr1::unordered_map<int, NodeX*>::const_iterator iter = pg->getNodesIterator();iter!=pg->getNodesEndIterator();++iter)
	{
		int varNodeID = iter->first;
		domains_values.insert(std::pair<int, tr1::unordered_set<int>*>(varNodeID, new tr1::unordered_set<int>()));
	}


	//insert domains values (considering node, neighborhood count, and degree consistencies)
	for(tr1::unordered_map<int, tr1::unordered_set<int>*>::iterator iter = domains_values.begin();iter!=domains_values.end();iter++)
	{
		//get the pattern node
		NodeX* pNode = pattern->getGraph()->getNodeWithID(iter->first);
		tr1::unordered_set<int>* currentDomain = domains_values[pNode->getID()];

		//check node consistency
		set<int>* nodes_SameLabel = graph->getNodesByLabel(pNode->getLabel());

		for(int iter1 : *nodes_SameLabel)
		{
			NodeX* dNode = graph->getNodeWithID(iter1);
			if(dNode->getEdgesSize() >= pNode->getEdgesSize())	// [*] apply neighborhood consistency
			{
				currentDomain->insert(dNode->getID());
			}
		}
		if(currentDomain->size()<support)
			return 0;
	}

	int patternSize = pattern->getGraph()->getNumOfNodes();
	long long start = getmsofday_3();

	//apply arc consistency constraint
	AC_3(graph, domains_values, pattern, support);

	long long end = getmsofday_3();
	long long elapsed = end - start;
	//check for domains lengths
	for(int i=0;i<patternSize;i++)
	{
		if(domains_values.find(i)->second->size()<support)
		{
			for(auto & domains_value : domains_values)
				delete domains_value.second;

			return 0;
		}
	}

	// [*] set domains order
	int* ordering  = new int[patternSize];

	for(int i=0;i<patternSize;i++)
	{
		ordering[i] = i;
	}

	//create solutions data structure
	for(int i=0;i<patternSize;i++)
	{
        domains_solutions[i] = std::set<int>();
	}

    // 使用unique labels优化策略
    //check for unique labels optimization, if applicable then return true (as we already checked in the previous  step for infrequentness)
    if (pattern->hasUniqueLabels() && GraMiCounter::isItAcyclic(*(pattern->getGraph())))//GraMiCounter::useAC3
    {
        int min = std::numeric_limits<int>::max();
        bool flag = false;
        for (auto & domains_value : domains_values)
        {
            flag = true;
            int temp = domains_value.second->size();
            //delete iter->second;//new
            if (temp < min)
                min = temp;
        }
        if(!flag) {
            min = 0;
        }
        if (min >= support) {
            for (auto & domains_value : domains_values)
            {
                for (int x : *domains_value.second) {
                    domains_solutions[domains_value.first].insert(x);
                }
            }
        }
        for (auto & domains_value : domains_values)
        {
            delete domains_value.second;
        }

        return min;
    }

	for(int i=0;i<patternSize;i++)
	{
		// domainID is the subgraph's node which is being processed（The domain of this node is currently being processed）
		int domainID = ordering[i];

		// [*] apply automorphism

		//go over elements in the current domain, check if a solution exists
		// currentDomain is the domain corresponding to the subgraph node currently being processed
		tr1::unordered_set<int>* currentDomain = domains_values.find(domainID)->second;

		if(Settings::debugMSG)
			cout<<"old currentDomain size:"<<currentDomain->size()<<endl;

		if(approximate!=-1)
		{
			map<int ,int> idMap;
			int c = 0;
			for(tr1::unordered_set<int>::iterator iter = currentDomain->begin();iter!=currentDomain->end();iter++)
			{
				idMap.insert(std::pair<int, int>(c, *iter));
				c++;
			}
			delete currentDomain;
			domains_values.erase(domainID);
			currentDomain = new tr1::unordered_set<int>();
			domains_values.insert(std::pair<int, tr1::unordered_set<int>* >(domainID, currentDomain));

			// allows approximate mining
			// Randomly sample "size()*approximate" number of samples in the original domain,
            // and redefine currentDomain to determine whether the domain meets the support requirements
			while(currentDomain->size()<(idMap.size()*approximate))
			{
				int r = rand()%idMap.size();
				currentDomain->insert(idMap.at(r));
			}
		}

		if(Settings::debugMSG)
			cout<<"new currentDomain size:"<<currentDomain->size()<<endl;

		int counter = 0;
		for(tr1::unordered_set<int>::iterator iter = currentDomain->begin();iter!=currentDomain->end();++iter)
		{
			//cout<<"DID:"<<domainID<<", c:"<<counter<<endl;
			counter++;
			// nodeID is the large graph node in the domain
            // corresponding to the subgraph domainID currently being processed
			int nodeID = (*iter);
			bool b = false;
			vector<map<int, int>* > result;
			//check if this node has been passed previously as a correct solution or not
			if(domains_solutions.find(domainID)->second.find(nodeID)!=domains_solutions.find(domainID)->second.end())
			{
				b = true;
			}
			else
			{
				pattern->getGraph()->isIsomorphic(graph, result, domains_values, domainID, nodeID);
				if(result.size()>0)
					b = true;
				else
				{
					//in case the remaining + existing solutions can not satisfy the support, break
					if(approximate==-1)
					{
						if(currentDomain->size()-counter+domains_solutions.find(domainID)->second.size()<support)
						{
							deleteResults(result);
							// In the case where approximate mining is not allowed,
                            // if the sum of (nodes to be detected + valid nodes) < support degree,
                            // the corresponding subgraph is infrequent
							break;
						}
					}
					else
					{
						if(currentDomain->size()-counter+domains_solutions.find(domainID)->second.size()<(support*approximate))
						{
							deleteResults(result);
							break;
						}
					}
				}
			}
			if(b)	//find valid node in currentDomain
			{
				if(approximate==-1)
				{
					//there is a solution for this node, add all valid node values to the solutions domain
					for(auto currentMapping : result)
					{
							// Traverse all the mapping nodes on the large graph that are isomorphic to the subgraph nodes,
                            // and insert them into domain_solution
                            // (mark all the values in the solution in the corresponding domain)
						for(auto & iter2 : *currentMapping)
						{
							int dID = iter2.first;
							int nID = iter2.second;
                            if (domains_solutions.find(dID) == domains_solutions.end()) {
                                domains_solutions[dID] = set<int>();
                            }
							domains_solutions[dID].insert(nID);
						}
					}
				}
				else
				{
                    if (domains_solutions.find(domainID) == domains_solutions.end()) {
                        domains_solutions[domainID] = set<int>();
                    }
                    domains_solutions[domainID].insert(nodeID);
				}
			}
			if(approximate==-1)
			{
				if(domains_solutions[domainID].size()>=support)
				{
                    if(Settings::debugMSG)
                        for (auto domain_solution: domains_solutions)
                            cout << domain_solution.second.size() << endl;
					deleteResults(result);
					break;
				}
			}
			else
			{
				if(domains_solutions[domainID].size()/approximate>=support)
				{
					deleteResults(result);
					break;
				}
			}

			deleteResults(result);
		}

		//in case the solution can not satisfy the support, break
		if(approximate==-1)
		{
			if(domains_solutions[domainID].size()<support)
				break;
		}
		else
		{
			int tf = domains_solutions[domainID].size()/approximate;
			if(tf<support)
			{
				break;
			}
		}
	}

    if(Settings::debugMSG) {
        cout << *pattern->getGraph();
        for (auto &domains_solution: domains_solutions) {
            cout << "domain ID: " << domains_solution.first << "domain size: " << domains_solution.second.size()
                 << endl;
            for (auto &value_iter: domains_solution.second) {
                cout << "value ID: " << value_iter << endl;
            }
        }
    }

	//delete the domains
	for(tr1::unordered_map<int, tr1::unordered_set<int>*>::iterator iter = domains_values.begin();iter!=domains_values.end();iter++)
	{
		delete iter->second;
	}

	//get MNI frequency using the domains solutions
	if(approximate==-1)
	{
		int min = std::numeric_limits<int>::max();
        bool flag = false;
		for(auto & domains_solution : domains_solutions)
		{
            flag = true;
			int temp = domains_solution.second.size();
			if(temp<min)
				min = temp;
		}
        if(!flag) {
            freq = 0;
        } else {
            freq = min;
        }

	}
	else //in case of approximation
	{
		int min = std::numeric_limits<int>::max();
		for(auto & domains_solution : domains_solutions)
		{
			int domainID = domains_solution.first;
			int temp = domains_solution.second.size();
			//temp = domains_values.find(domainID)->second->size();
			temp = temp / approximate;
			if(temp<min)
				min = temp;
		}
		freq = min;
	}


	//delete domains solutions
//	for(auto & domains_solution : domains_solutions)
//	{
//		domains_solution.second.clear();
//	}

	//delete the ordering
	delete ordering;

	return freq;
}


void GraMiCounter::AC_3(GraphX* graph, tr1::unordered_map<int, tr1::unordered_set<int>*>& domains_values, Pattern* pattern, int support)
{
	GraMiCounter::AC_3(graph, domains_values, pattern->getGraph(), support, pattern->getInvalidCol());
}

void GraMiCounter::AC_3(GraphX* graph, tr1::unordered_map<int, tr1::unordered_set<int>*>& domains_values, GraphX* pGraph, int support, int invalidCol)
{
	map<string,pair_with_edge*> arcs;

	//create pairs of connection from the pattern
	for(tr1::unordered_map<int, NodeX*>::const_iterator iter = pGraph->getNodesIterator();iter!=pGraph->getNodesEndIterator();++iter)
	{
		NodeX* pNode = iter->second;
		for(tr1::unordered_map<int, void*>::iterator iter1 = pNode->getEdgesIterator();iter1!=pNode->getEdgesEndIterator();iter1++)
		{
			EdgeX* edge = (EdgeX*)(iter1->second);
			int id1 = pNode->getID();
			int id2 = edge->getOtherNode()->getID();
			string sig = getSig(id1, id2, edge->getLabel());

			if(arcs.find(sig)!=arcs.end())
				continue;

			pair_with_edge* pwe = new pair_with_edge();
			pwe->id1 = id1;
			pwe->id2 = id2;
			pwe->edgeLabel = edge->getLabel();
			pwe->minDomainSize = domains_values.find(pwe->id1)->second->size();
			if(pwe->minDomainSize<domains_values.find(pwe->id2)->second->size())
				pwe->minDomainSize=domains_values.find(pwe->id2)->second->size();

			auto iter = arcs.begin();
			for(;iter!=arcs.end();iter++)
			{
				pair_with_edge* temp_pwe = iter->second;
				//cout<<iter->first<<endl<<flush;
				if(temp_pwe->minDomainSize>pwe->minDomainSize)
					break;
			}
			if(arcs.find(sig)!=arcs.end())//new 2April
				delete pwe;
			else
				arcs.insert(iter, std::pair<string, pair_with_edge*>(sig, pwe));
		}
	}

	//order arcs to start with invalidcolumn if exists

	while(arcs.size()>0)
	{
		pair_with_edge* pwe = 0;
		string sig;

		if(invalidCol!=-1)
		{
			auto iter = arcs.begin();
			for(;iter!=arcs.end();iter++)
			{
				pair_with_edge* temp_pwe = iter->second;
				if(temp_pwe->id1==invalidCol || temp_pwe->id2==invalidCol)
				{
					pwe = iter->second;
					sig = iter->first;
					break;
				}
			}
		}

		if(pwe==0)
		{
			pwe = arcs.begin()->second;
			sig = arcs.begin()->first;
		}

		//this lookup is repeated in the refine function, think of a way not to repeat it
		tr1::unordered_set<int>* D1 = domains_values.find(pwe->id1)->second;
		tr1::unordered_set<int>* D2 = domains_values.find(pwe->id2)->second;
		int old_size1 = D1->size();
		int old_size2 = D2->size();

		if(refine(graph, D1, D2, pwe, support))
		{
			auto iter = arcs.begin();
			for(;iter!=arcs.end();iter++)
			{
				pair_with_edge* temp_pwe = iter->second;
				delete temp_pwe;
			}
			arcs.clear();
			return;
		}

		//add affected arcs
		NodeX* pNode = pGraph->getNodeWithID(pwe->id1);
		if(old_size1!=D1->size() && pNode->getEdgesSize()>1)
		{
			int id1 = pwe->id1;
			for(tr1::unordered_map<int, void*>::iterator iter1 = pNode->getEdgesIterator();iter1!=pNode->getEdgesEndIterator();iter1++)
			{
				EdgeX* edge = (EdgeX*)(iter1->second);

				int id2 = edge->getOtherNode()->getID();
				if(id2==pwe->id2)
					continue;
				string sig = getSig(id1, id2, pwe->edgeLabel);

				if(arcs.find(sig)!=arcs.end())
					;//delete pwe;
				else
				{
					//create pwe
					pair_with_edge* pwe = new pair_with_edge();
					pwe->id1 = id1;
					pwe->id2 = id2;
					pwe->edgeLabel = edge->getLabel();
					pwe->minDomainSize = domains_values.find(pwe->id1)->second->size();
					if(pwe->minDomainSize<domains_values.find(pwe->id2)->second->size())
					pwe->minDomainSize=domains_values.find(pwe->id2)->second->size();

					arcs.insert(std::pair<string, pair_with_edge*>(sig, pwe));
				}
			}
		}

		pNode = pGraph->getNodeWithID(pwe->id2);
		if(old_size2!=D2->size() && pNode->getEdgesSize()>1)
		{
			int id1 = pwe->id2;
			for(tr1::unordered_map<int, void*>::iterator iter1 = pNode->getEdgesIterator();iter1!=pNode->getEdgesEndIterator();iter1++)
			{
				EdgeX* edge = (EdgeX*)(iter1->second);

				int id2 = edge->getOtherNode()->getID();
				if(id2==pwe->id1)
					continue;
				string sig = getSig(id1, id2, pwe->edgeLabel);

				if(arcs.find(sig)!=arcs.end())
					;//delete pwe;
				else
				{
					//create pwe
					pair_with_edge* pwe = new pair_with_edge();
					pwe->id1 = id2;
					pwe->id2 = id1;
					pwe->edgeLabel = edge->getLabel();
					pwe->minDomainSize = domains_values.find(pwe->id1)->second->size();
					if(pwe->minDomainSize<domains_values.find(pwe->id2)->second->size())
					pwe->minDomainSize=domains_values.find(pwe->id2)->second->size();

					arcs.insert(std::pair<string, pair_with_edge*>(sig, pwe));
				}
			}
		}

		arcs.erase(sig);
		delete pwe;
	}

	//clear arcs list
	auto iter = arcs.begin();
	for(;iter!=arcs.end();iter++)
	{
		pair_with_edge* temp_pwe = iter->second;
		delete temp_pwe;
	}
	arcs.clear();
}

/**
 * This method requires that for the smaller domain D1, all nodes in it must satisfy:
 * be able to find a node in D2 that is connected to this node by the edge with "edgeLabel",
 * otherwise it should be deleted the node in D1.
 *
 * At the same time, the node in D2 should also satisfy that a node can be found connected to it in D1
 * and the edge label is edgeLabel, otherwise it is also to be deleted
 *
 * After the above deletion,
 * if the domain size of either of D1 and D2 is less than the required support,
 * then return false, otherwise return true
 * @param graph
 * @param D1
 * @param D2
 * @param pwe
 * @param support
 * @return
 */
bool GraMiCounter::refine(GraphX* graph, tr1::unordered_set<int>* D1, tr1::unordered_set<int>* D2, pair_with_edge* pwe, int support)
{
    double edgeLabel = pwe->edgeLabel;
	//set to iterate over the smaller domain
	if(D1->size()>D2->size())
	{
		tr1::unordered_set<int>* temp = D1;
		D1 = D2;
		D2 = temp;
	}
	//go over each node in the domain1
	set<int> new_D2;
	for(tr1::unordered_set<int>::iterator iter = D1->begin();iter!=D1->end();)
	{
		NodeX* node = graph->getNodeWithID((*iter));
		//go over its edges
		bool deleteIt = true;
		for(tr1::unordered_map<int, void*>::iterator iter1 = node->getEdgesIterator();iter1!=node->getEdgesEndIterator();++iter1)
		{
			EdgeX* edge = (EdgeX*)(iter1->second);
			if(edge->getLabel()!=edgeLabel)
				continue;
			tr1::unordered_set<int>::iterator iter_f = D2->find(edge->getOtherNode()->getID());
			if(iter_f!=D2->end())
			{
				// A node is found in D2 which is connected to the node in D1 by an edge labeled "edgeLabel"
				deleteIt = false;
				new_D2.insert(*iter_f);
			}
		}

		if(deleteIt)
		{
			iter = D1->erase(iter);
			if(D1->size()<support)
				return true;
		}
		else
			++iter;
	}

	for(tr1::unordered_set<int>::iterator iter = D2->begin();iter!=D2->end();)
	{
		if(new_D2.find(*iter)==new_D2.end())
		{
			iter = D2->erase(iter);
			if(D2->size()<support)
				return true;
		}
		else
		{
			iter++;
		}
	}

	return false;
}

/**
 * given the graph, check whether it is Acyclic or not (we assume the graph is connected)
 * @param me
 * @return
 */
bool GraMiCounter::isItAcyclic(GraphX& query)
{
	tr1::unordered_set<int> visited;
	vector<int> toBeVisited;

	int currentNodeID;
	NodeX* currentNode;
	toBeVisited.push_back(0);
	while(visited.size()<query.getNumOfNodes())
	{
		if(toBeVisited.size()==0)
			break;

		currentNodeID = toBeVisited.at(0);
		currentNode = query.getNodeWithID(currentNodeID);

		toBeVisited.erase(toBeVisited.begin());
		visited.insert(currentNodeID);
		//get all neighbor nodes (incoming and outgoing)
		int alreadyVisitedNeighbors = 0;//this should not be more than 1

		//all edges!
		for(tr1::unordered_map<int, void*>::iterator iter = currentNode->getEdgesIterator();iter!=currentNode->getEdgesEndIterator();iter++)
		{
			EdgeX* edge = (EdgeX*)iter->second;
			int otherNodeID = edge->getOtherNode()->getID();

			if(visited.find(otherNodeID)!=visited.end())
			{
				alreadyVisitedNeighbors++;
				if(alreadyVisitedNeighbors>1)
				{
					//cout<<"It is CYCLIC!";
					return false;
				}
			}
			else
			{
				toBeVisited.push_back(otherNodeID);
			}
		}
	}

	return true;
}

/**
 * Populate the ordering 'order' from domains with smaller sizes to those with larger sizes.、
 * 按照域的大小从小到大排序
 */
void GraMiCounter::setDomainsOrder(tr1::unordered_map<int, tr1::unordered_set<int>*>& domains_values, int* order, Pattern* pattern)
{
	int invalidCol;
	if(Settings::usePredictedInvColumn)
		invalidCol = pattern->getInvalidCol();
	else
		invalidCol = -1;

	tr1::unordered_set<int> added;
	int i = 0;
	// If there is a column that is predicted to be invalid,
    // put it first (first detection, seeking earlier pruning)
	if(invalidCol>-1)
	{
		order[0] = invalidCol;
		added.insert(invalidCol);
		i = 1;
	}

	for(;i<domains_values.size();i++)
	{
		//find the minimum
		int min = std::numeric_limits<int>::max();
		int ID4min = -1;

		for(tr1::unordered_map<int, tr1::unordered_set<int>*>::iterator iter = domains_values.begin();iter!=domains_values.end();iter++)
		{
			//get the pattern node
			int currentID = iter->first;
			tr1::unordered_set<int>* currentSet = iter->second;

			// The size of the domain corresponding to the node / the number of edges connected to the node
			long currentDomainValue = currentSet->size()/pattern->getGraph()->getNodeWithID(currentID)->getEdgesSize();

			if(currentDomainValue<min && added.find(currentID)==added.end())
			{
				ID4min = currentID;
				min = currentDomainValue;
			}
		}

		// Each time the smallest remaining is found, insert
		order[added.size()] = ID4min;
		added.insert(ID4min);
	}

	if(Settings::debugMSG)
	{
		for(int ii=0;ii<domains_values.size();ii++)
			cout<<order[ii]<<", ";
		cout<<endl;
	}
}
