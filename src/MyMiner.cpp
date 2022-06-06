/**
*
MyMiner.cpp
 */

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include "MyMiner.h"
#include "utils.h"
#include "Settings.h"
#include "GraMiCounter.h"

/**
 * start the mining process, given the filename (file base name for the partitions)
 * , and the required support threshold
 */
void MyMiner::startMining(string fileName, int graphType, int support, int givenType) {
    int numberOfCreatedCandids = 0;

    numOfVisitedNodes = 0;
    numIterations = 0;

    this->support = support;

    //load the graph
    long long start = getmsofday();
    loadGraph(fileName, graphType, support, frequentEdges);

    if (frequentEdges.getSize() == 0) {
        cout << "No frequent patterns found! Exiting" << endl;
        exit(0);
    }

    long long end = getmsofday();
    long long elapsed = end - start;

    cout << "Loading took " << (elapsed / 1000) << " sec and " << (elapsed % 1000) << " ms" << endl;


//    Settings::graphLoadingTime = elapsed;

    CLMap *clmap = new CLMap();

    if (givenType == -1) {
        clmap->addAll(&frequentEdges);
    }
        // Restrict the subgraph must contain nodes of type givenType
    else {
        auto iter = frequentEdges.getFirstElement();
        while (iter.pattern != 0) {
            GraphX g = iter.pattern->getGraph();
            if (g.getNodeWithID(0)->getLabel() != givenType && g.getNodeWithID(1)->getLabel() != givenType) {
                frequentEdges.removePattern(iter.pattern);
            }
            frequentEdges.advanceIterator(iter);
        }
    }

    frequentPatterns.insert(frequentPatterns.begin() + 1, clmap);//frequent edges!

    if (Settings::debugMSG) {
        cout << "#frequent edges  = " << frequentEdges.getSize() << endl;
        cout << "Frequent edges:" << endl;
        frequentEdges.print();
    }

    //extend frequent edges into 2 edges graphs

    start = getmsofday();

    extendFreqEdges();

    end = getmsofday();

    cout << "Start mining ..." << endl;

    tr1::unordered_set<int> delete_pattern_id = tr1::unordered_set<int>();

    while (true) {
        //check if we have more candidates with the same current size
        if (currentlyChecking.size() == 0 && getNumElems((vector<map<string, void *> *> *) &candidates) == 0) {
            break;
        }

        //send available candidates to frequency counting
        bool isFreq = false;
        domains_solutions.clear();
        char canInfo[50];
        strcpy(canInfo, popMyCandidate(candidates, currentlyChecking, support, -1));

        int res, candidateID = 0;
        sscanf(canInfo, "%d,%d", &res, &candidateID);
        if (res == 1) {
//            for (auto domain_solution : domains_solutions) {
//                cout << domain_solution.second.size() <<endl;
//            }
            isFreq = true;
            if (Settings::debugMSG) {
                cout << "frequent!" << endl;
                cout << canInfo << endl;
            }
        }

        if (isFreq) {
            Pattern *candidate = currentlyChecking.find(candidateID)->second;
            candidate->setFrequency(support);
            currentlyChecking.erase(candidateID);
            if ((frequentPatterns.size() - 1) < candidate->getSize()) {
                frequentPatterns.insert(frequentPatterns.begin() + candidate->getSize(), new CLMap());
            }
            frequentPatterns[candidate->getSize()]->addPattern(candidate);
            frequentPatternVec.emplace_back(candidate);
//            frequentPatternsDomain[frequentPatternVec.size()] = candidate->getDomainValues();

            if (Settings::debugMSG) {
                cout << "insert into frequentPatternsDomain" << endl;
                cout << "count: " << frequentPatternVec.size()-1 << endl;
                for (auto & domain : domains_solutions) {
                    cout << "domain ID: " << domain.first << endl;
                    for (auto & value : domain.second) {
                        cout << "value ID: " << value <<endl;
                    }
                }
            }

            frequentPatternsDomain[frequentPatternVec.size()-1] = domains_solutions;
//            frequentPatternsDomain.insert(std::pair<int, map<int, set<int>>>(frequentPatternVec.size(), domains_solutions));
            if (Settings::debugMSG)
                cout << "size of frequentPatternsDomain: " << frequentPatternsDomain.size() << endl;

            //extend using the current frequent patterns with the same size
            if (Settings::debugMSG)
                cout << "extending based on current frequent pattern" << endl;
            CLMap_Iterator iter1 = frequentPatterns[candidate->getSize()]->getFirstElement();
            while (iter1.pattern != 0) {
                Pattern *temp = iter1.pattern;//->second;
                frequentPatterns[candidate->getSize()]->advanceIterator(iter1);

                set<std::pair<PrimaryGraph *, PrimaryGraph *> > joiningPGs = candidate->getJoiningPG(temp);
                for (const auto &joiningPG: joiningPGs) {
                    PrimaryGraph *candPG = joiningPG.first;
                    PrimaryGraph *otherPG = joiningPG.second;

                    //find isomorphisms
                    vector<map<int, int> *> result;
                    tr1::unordered_map<int, tr1::unordered_set<int> *> domains_values;
                    otherPG->getGraph()->isIsomorphic(candPG->getGraph(), result, domains_values);

                    //for each result
                    for (auto &iter2: result) {
                        map<int, int> *currResult = iter2;

                        //add the new edge
                        int scrNodeID = currResult->find(otherPG->getSrcNodeID())->second;
                        EdgeX *edge = otherPG->getEdge();
                        GraphX *newGraph = new GraphX(candidate->getGraph());
                        int newID = newGraph->getNumOfNodes();
                        while (newGraph->getNodeWithID(newID) != NULL) {
                            newID++;
                        }
                        newGraph->AddNode(newID, edge->getOtherNode()->getLabel());
                        newGraph->addEdge(scrNodeID, newID, edge->getLabel());
                        //create the new candidate object
                        Pattern *newCandidate = new Pattern(newGraph, false);
                        if (candidates.size() <= newCandidate->getSize()) {
                            candidates.insert(candidates.begin() + newCandidate->getSize(), new CLMap());
                        }
                        newCandidate->generatePrimaryGraphs();
                        bool b = candidates.at(newCandidate->getSize())->addPattern(newCandidate);
                        if (!b) delete newCandidate;
                        //if the two edges have the same label, we need to assume the same label belong to one node
                        if (candPG->getEdge()->getOtherNode()->getLabel() ==
                            otherPG->getEdge()->getOtherNode()->getLabel() &&
                            candPG->getSrcNodeID() != scrNodeID) {
                            newGraph = new GraphX(candidate->getGraph());
                            newGraph->addEdge(candPG->getEdge()->getOtherNode()->getID(), scrNodeID,
                                              otherPG->getEdge()->getLabel());
                            newCandidate = new Pattern(newGraph, false);
                            newCandidate->generatePrimaryGraphs();
                            //CHECK THIS LINE IS TAKING SO MUCH TIME WHEN THE PATTERN IS BIG!!!! ---->
                            bool b = candidates.at(newCandidate->getSize())->addPattern(newCandidate);
                            if (!b) delete newCandidate;
                        }
                        delete iter2;
                    }
                }
            }
        } else {
            //cout << "find candidate and remove" << endl;
            Pattern *candidate = currentlyChecking.find(candidateID)->second;
            candidate->setFrequency(0);
            //cout << "erase" << endl;
            currentlyChecking.erase(candidateID);
        }
    }

    if (Settings::debugMSG) {
        cout << "size of ultimate frequentPatternsDomain: " << frequentPatternsDomain.size() << endl;
        cout << "now print all frequentPatternsDomain!" << endl;
        cout << "test1" << endl;
        printDomains();
//        for (auto &pattern_iter: frequentPatternsDomain) {
//            cout << *(frequentPatternVec[pattern_iter.first]->getGraph()) << endl;
//            for (auto &map_iter: pattern_iter.second) {
//                cout << "domain: " << map_iter.first << endl;
//                cout << "values: " << endl;
//                set<int> st = map_iter.second;
//                for (auto &value_iter: st) {
//                    cout << value_iter << endl;
//                }
//            }
//        }
        cout << "remove partial graph" << endl;
    }

//    now delete invalid frePatterns
    int pattern_cnt = 0;
    for (auto frePattern : frequentPatternVec) {
        if (Settings::debugMSG) {
            cout << "test this subgraph!" << endl;
            cout << *(frePattern->getGraph()) << endl;
        }
        bool del = false;
        printDomains();
        auto domainNodes = frequentPatternsDomain[pattern_cnt];
        if (!domainNodes.empty()) {
            if (Settings::debugMSG) {
                cout << "subgraph found!";
            }
        }
        else {
            cout << "empty domainNodes" << pattern_cnt  << endl;
        }

        if (Settings::debugMSG) {
            for (auto &domainNode: domainNodes) {
                int value_num = 0;
                cout << "domain ID: " << domainNode.first << endl;
                for (int value_iter: domainNode.second) {
                    cout << "value ID" << value_num << ": " << value_iter << endl;
                    value_num++;
                }
            }
        }

        GraphX *pg = frePattern->getGraph();
        for (auto &nodeIter1: domainNodes) {
            NodeX *pNode = frePattern->getGraph()->getNodeWithID(nodeIter1.first);
            set<int> nodeSet = nodeIter1.second;
            vector<int> linkedDomain;
            vector<int> otherDomain;
            // 得到pNode的邻接结点linkedDomain
            for (auto edgeIter1 = pNode->getEdgesIterator(); edgeIter1 != pNode->getEdgesEndIterator(); ++edgeIter1) {
                // linkedID: the other nodeID of the edge
                int linkedID = edgeIter1->first;
                linkedDomain.push_back(linkedID);
            }
            // 不与pNode邻接的结点otherDomain
            for (auto nodeIter2 = pg->getNodesIterator(); nodeIter2 != pg->getNodesEndIterator(); ++nodeIter2) {
                if (nodeIter2->second->getID() == pNode->getID()) {
                    continue;
                }
                bool flag = false;
                for (int i : linkedDomain) {
                    if (nodeIter2->second->getID() == i) {
                        flag = true;
                        break;
                    }
                }
                if (!flag) {
                    otherDomain.push_back(nodeIter2->second->getID());
                }
            }

            int num1 = 0;
            if (Settings::debugMSG) {
                for (int &value_iter: linkedDomain) {
                    cout << "linked value ID" << num1 << ": " << value_iter << endl;
                    num1++;
                }
                int num2 = 0;
                for (int &value_iter: otherDomain) {
                    cout << "other value ID" << num2 << ": " << value_iter << endl;
                    num2++;
                }
            }

            for (auto id: nodeSet) {
                // 遍历pNode的域，当前结点为gNode
                NodeX *gNode = graph->getNodeWithID(id);
                if (gNode == nullptr) {
                    cout << "gNode not found!" << endl;
                    exit(0);
                } else {
                    if (Settings::debugMSG) {
                        cout << "find gNode!" << endl;
                    }
                }
                for (tr1::unordered_map<int, void *>::iterator edgeIter2 = gNode->getEdgesIterator();
                     edgeIter2 != gNode->getEdgesEndIterator(); ++edgeIter2) {
                    // gNode的邻接结点otherNodeID
                    int otherNodeID = edgeIter2->first;
                    bool flag1 = false;
                    for (int k: linkedDomain) {
                        auto linkedValues = domainNodes[k];
                        if (linkedValues.empty()) {
                            cout << "linkedDomain[k]" << k << " not found in domainNodes!";
                            exit(0);
                        } else {
                            if (Settings::debugMSG) {
                                cout << "linkedDomain[k]"<<k<<" found in domainNodes!";
                            }
                        }
                        // 与当前gNode相连的节点不在pNode的相邻节点的域中
                        if (linkedValues.find(otherNodeID) != linkedValues.end()) {
                            // 当前的otherNode通过审核
                            break;
                        }
                        for (int j: otherDomain) {
                            auto otherValues = domainNodes[j];
                            if (otherValues.empty()) {
                                cout << "otherValues[k] not found in otherValues!";
                                exit(0);
                            } else {
                                if (Settings::debugMSG) {
                                    cout << "otherValues[k] found in otherValues!";
                                }
                            }
//                                set<int>* otherValues = domainNodes[otherDomain[j]];
                            // 与当前gNode相连的节点却存在于pattern中其他结点的域中，说明当前的pattern中结点的联系没有被完全表示，不符合module的要求
                            if (otherValues.find(otherNodeID) != otherValues.end()) {
                                flag1 = true;
                                break;
                            }
                        }
                        if (flag1) {
                            del = true;
                            break;
                        }
                    }
                    if (del) {
                        break;
                    }
                }
                if (del) {
                    break;
                }
            }
            if (del) {
                break;
            }
        }
        if (del) {
//            frequentPatterns[frePattern->getSize()]->removePattern(frePattern);
//            frequentPatternVec.erase(pattern_iter);
            if (Settings::debugMSG) {
                cout << "delete this graph" << endl;
                cout << *(frePattern->getGraph()) << endl;
            }
            delete_pattern_id.insert(pattern_cnt);
        }
        pattern_cnt++;
    }
    printResult(delete_pattern_id);
}

char* MyMiner::popMyCandidate(vector<CLMap*>& candidates, map<int, Pattern*>& currentlyChecking, int support, double approximate)
{
    if (Settings::debugMSG)
        cout << "try to pop a candidate ..." << endl;
    static char result[50];
    bool isFreq = false;
    CLMap* currentCandidates = 0;
    for (auto & candidate : candidates)
    {
        if (candidate->getSize() != 0)
        {
            currentCandidates = candidate;
            break;
        }
    }

    if (currentCandidates == 0)
    {
        if (Settings::debugMSG)
            cout << "No more candidate ..." << endl;
        result[0] = '0';
        return result;
    }

    if (Settings::debugMSG)
        cout << "Popping a candidate ..." << endl;
    //get a candidate, if there is
    if (currentCandidates->getSize() > 0)
    {
        CLMap_Iterator iter = currentCandidates->getFirstElement();
        string key = iter.key;
        Pattern* candidate = iter.pattern;
        currentCandidates->removePattern(candidate);

        // sendACandidateApprox(key, candidate, currentlyChecking, destination);
        if (currentlyChecking.find(candidate->getID()) != currentlyChecking.end())
        {
            cout << "candidate->getID() = " << candidate->getID() << endl << flush;
            exit(0);
        }

        if (Settings::debugMSG) {
            cout << "candidate ID: " << candidate->getID() << endl;
        }
        currentlyChecking[candidate->getID()] = candidate;
//		currentlyChecking.insert(std::pair<int, Pattern*>(candidate->getID(), candidate));

        Miner::numIsFreqCalls++;

        ostringstream tempOS;
        tempOS << *(candidate->getGraph());
        char graphStr[2000];
        sprintf(graphStr, "%d,%s\t", candidate->getID(),tempOS.str().c_str());
        if (Settings::debugMSG)
            cout << "now pattern: " << graphStr << endl;

        isFreq = workCount(graphStr, support, approximate);
        int fre = isFreq ? 1 : 0;
        sprintf(result, "%d,%d", fre, candidate->getID());
    }

    return result;
}

void clearAllMP(tr1::unordered_map<string, void* >& allMP)
{
    for(tr1::unordered_map<string, void* >::iterator iter = allMP.begin();iter!=allMP.end();iter++)
    {
        delete iter->second;
    }
    allMP.clear();
}

bool MyMiner::workCount(char* graphStr, int support, double approximate) {
    tr1::unordered_map<string, void* > allMPs;
    GraphX* subgraph = new GraphX(1, false);

    int candidateID;
    char subgraphTemp[500];
    sscanf (graphStr,"%d,%[^\t]", &candidateID, subgraphTemp);
    std::stringstream sstmGF;
    sstmGF << subgraphTemp;
    string subgraphStr = sstmGF.str();
    subgraph->loadFromString(subgraphStr, allMPs);
    clearAllMP(allMPs);
    Pattern* newCandidate = new Pattern(subgraph, false);

    unsigned long startTime = getmsofday();
    int freq = getFreq(newCandidate, support, approximate);
    bool isFreq;
    if (freq >= support) {
        isFreq = true;
//        cout << "test in fuc workCount" << endl;
//        cout << *subgraph;
//        for (auto & domains_solution : domains_solutions) {
//            cout << "domain ID: " << domains_solution.first << "domain size: " << domains_solution.second.size() << endl;
//            for (auto & value_iter : domains_solution.second) {
//                cout << "value ID: " << value_iter;
//            }
//        }
    }
    else
        isFreq = false;

    unsigned long elapsed = getmsofday() - startTime;

    if (Settings::debugMSG)
        printf("Time taken is: %Lu. Predicted total time is: %lu\n", elapsed);

    delete subgraph;
    delete newCandidate;
    if (isFreq && Settings::debugMSG) {
        cout << "workCount True!" << endl;
    }
    return isFreq;
}


void MyMiner::printTotalExpectedTime()
{
    vector<CLMap* >* allPatterns[2];
    allPatterns[0] = &frequentPatterns;
    allPatterns[1] = &infrequentPatterns;

    unsigned long int totatTime = 0;

    //add to candidates
    for (int l = 0; l < 2; l++)
    {
        for (vector<CLMap* >::iterator iter = allPatterns[l]->begin(); iter != allPatterns[l]->end(); iter++)
        {
            CLMap* tempMap = (*iter);
            CLMap_Iterator iter1 = tempMap->getFirstElement();
            while (iter1.pattern != 0)
            {
                Pattern* pattern = iter1.pattern;//->second;
                tempMap->advanceIterator(iter1);
                totatTime += pattern->getPredictedTime();
            }
        }
    }
}

int MyMiner::getFreq(Pattern* candidate, int support, double approximate)
{
    //cout << "now counting freq!" << endl;
    vector<unsigned long> listOfNumOfIters;
    int freq = 0;
    freq = GraMiCounter::isFrequent(graph, candidate, support, approximate, domains_solutions);
    if (Settings::debugMSG) {
        if (freq >= support) {
            cout << *candidate->getGraph();
            for (auto &domains_solution: domains_solutions) {
                cout << "domain ID: " << domains_solution.first << "domain size: " << domains_solution.second.size()
                     << endl;
                for (auto &value_iter: domains_solution.second) {
                    cout << "value ID: " << value_iter;
                }
            }
        }
    }
    return freq;
}


void MyMiner::printResult(tr1::unordered_set<int> delete_pattern_id) {
    cout << "print final result!";
    int cnt = 0;
    for (auto & pattern : frequentPatternVec) {
        if (delete_pattern_id.find(cnt)!=delete_pattern_id.end()) {
            cnt++;
            continue;
        }
        cout << endl;
        cout << "*****frequent pattern " << cnt << "*****" << endl;
        cout << *(frequentPatternVec[cnt]->getGraph()) << endl;
        auto now_domain = frequentPatternsDomain[cnt];
        for (auto &map_iter: now_domain) {
            cout << "domain: " << map_iter.first << endl;
            cout << "values: " << endl;
            set<int> st = map_iter.second;
            for (auto &value_iter: st) {
                cout << value_iter << endl;
            }
        }
        cout << "**************************" << endl;
        cnt++;
    }
}

void MyMiner::printDomains() {
    for (auto &pattern_iter: frequentPatternsDomain) {
        cout << "subgraph" << pattern_iter.first << ":" << endl;
        cout << *(frequentPatternVec[pattern_iter.first]->getGraph()) << endl;
        for (auto &map_iter: pattern_iter.second) {
            cout << "domain: " << map_iter.first << endl;
            cout << "values: " << endl;
            set<int> st = map_iter.second;
            for (auto &value_iter: st) {
                cout << value_iter << endl;
            }
        }
    }
}
