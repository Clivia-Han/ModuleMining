#include <iostream>
#include <sstream>
#include<sys/time.h>
#include <stdlib.h>
#include <string.h>
#include "Miner.h"
#include "utils.h"

long long getmsofday_2()
{
   struct timeval tv;
   struct timezone tz;
   gettimeofday(&tv, &tz);
   return (long long)tv.tv_sec*1000 + tv.tv_usec/1000;
}

int Miner::numIsFreqCalls = 0;
int Miner::numFreqs = 0;
int Miner::numInfreq = 0;

Miner::Miner()
{
	candidates.insert(candidates.begin(), new CLMap());//graphs with zero edges!
	candidates.insert(candidates.begin()+1, new CLMap());//graphs with one edge! no ned also, because we have special treatment for graphs with one edge

	frequentPatterns.insert(frequentPatterns.begin(), new CLMap());//frequent patterns with zero edges!

	infrequentPatterns.insert(infrequentPatterns.begin(), new CLMap());//infrequents with zero edges!
	infrequentPatterns.insert(infrequentPatterns.begin()+1, new CLMap());//infrequents with one edge!
}

void Miner::loadGraph(string baseName, int graphType, int support, CLMap& freqEdges)
{
	this->support = support;

	tr1::unordered_map<string, void* > allMPs;

	int nodesCounter = 0;

	//load graph data
	std::stringstream sstmGF;
	sstmGF << baseName;
	string partFileName = sstmGF.str();
	graph = new GraphX(1, graphType);//create a graph with id=its partition id
	graph->setFrequency(support);
	cout<<"Master: created a graph object"<<endl;
	if(!graph->loadFromFile(partFileName, allMPs))
	{
		cout<<"Master: delete a graph object"<<endl;
		delete graph;
	}
	nodesCounter+=graph->getNumOfNodes();

	cout<<"Master: loop starts"<<endl;
	//遍历所有边，将频繁边加入freqEdges
	for(tr1::unordered_map<string, void* >::iterator iter = allMPs.begin(); iter!=allMPs.end();++iter)
	{
		if(((Pattern*)((*iter).second))->getFrequency()>=support)
        {
            Pattern* p = (Pattern*)((*iter).second);
            cout<<*(p->getGraph())<<endl;
            freqEdges.addPattern(p);
            frequentPatternVec.push_back(p);
            frequentPatternsDomain.insert(std::pair<int, map<int, set<int>>>(frequentPatternVec.size()-1, p->getDomainValues()));
        }
		else
			delete (Pattern*)((*iter).second);
	}
	cout<<"Master: loop finishes"<<endl;

	cout<<" data loaded successfully!"<<endl;
}

void Miner::extendFreqEdges()
{
	CLMap* twoEdgesCandidate = new CLMap();
	candidates.insert(candidates.begin()+2, twoEdgesCandidate);

	long long totalElapsed = 0;

	CLMap_Iterator iter1 = frequentEdges.getFirstElement();
	while(iter1.pattern!=0)
	//for(map<string, Pattern*>::iterator iter1 = frequentEdges.begin();iter1!=frequentEdges.end();++iter1)
	{
		Pattern* edge1 = iter1.pattern;//->second;

		double l1_0 = edge1->getGraph()->getNodeWithID(0)->getLabel();
		double l1_1 = edge1->getGraph()->getNodeWithID(1)->getLabel();

		CLMap_Iterator iter2 = iter1.getCopy();
		while(iter2.pattern!=0)
		//for(map<string, Pattern*>::iterator iter2 = iter1;iter2!=frequentEdges.end();++iter2)
		{
			Pattern* edge2 = iter2.pattern;//->second;
			frequentEdges.advanceIterator(iter2);

			double l2_0 = edge2->getGraph()->getNodeWithID(0)->getLabel();
			double l2_1 = edge2->getGraph()->getNodeWithID(1)->getLabel();
			double edge2Label = edge2->getGraph()->getEdgeLabel(0,1);

			//connectType refers to the way they can connect, -q means they can not connect, 0 means 0 and 0, 1 means 0 and 1, 2 means 1 and 0, and 3 means 1 and 1
			int lastConnectType = -1;
			while(true)
			{
				int connectType = -1;

				//check how can they (edge1 and edge2) connect.
				if(l1_0==l2_0 && lastConnectType<0)
					connectType = 0;
				else if(l1_0==l2_1 && lastConnectType<1)
					connectType = 1;
				else if(l1_1==l2_0 && lastConnectType<2)
					connectType = 2;
				else if(l1_1==l2_1 && lastConnectType<3)
					connectType = 3;

				//could not find an extension by edge1 and edge2
				if(connectType==-1)
					break;

				lastConnectType = connectType;

				long long start = getmsofday_2();

				Pattern* candidate = new Pattern(edge1);
				candidate->invalidateFrequency();

				long long end = getmsofday_2();
				totalElapsed += (end-start);

				switch(connectType)
				{
				case 0://00
					candidate->extend(0, 2, l2_1, edge2Label);
					break;
				case 1://01
					candidate->extend(0, 2, l2_0, edge2Label);
					break;
				case 2://10
					candidate->extend(1, 2, l2_1, edge2Label);
					break;
				case 3://11
					candidate->extend(1, 2, l2_0, edge2Label);
					break;
				}

				if(twoEdgesCandidate->getPattern(candidate)!=0)
					delete candidate;
				else
				{
					candidate->generatePrimaryGraphs();
					cout<<*(candidate->getGraph())<<endl;
					bool b = twoEdgesCandidate->addPattern(candidate);
					if(!b) delete candidate;
				}
			}

		}

		frequentEdges.advanceIterator(iter1);
	}
}

void Miner::setFrequentEdges(CLMap& freqEdges)
{
	this->frequentEdges.addAll(&freqEdges);
}

void Miner::printCandidates()
{
	print(candidates);
}

void Miner::printResult()
{
	print(frequentPatterns);
}

void Miner::removePattern(Pattern* pattern, vector<CLMap* >& data)
{
	CLMap* tempList = data.at(pattern->getSize());
	tempList->removePattern(pattern);
}

void Miner::print(vector<CLMap* >& data)
{
	//count the total number of frequent patterns
	int size = 0;
	for(unsigned int i=0;i<data.size();i++)
	{
		size+=data[i]->getSize();
	}
	int count = 1;
	cout<<"[Miner] There are "<<size<<" frequent patterns, and they are:"<<endl;
	for(unsigned int i=0;i<data.size();i++)
	{
		cout<<"With "<<(i)<<" edges:"<<endl;
		data[i]->print();
	}
}

Miner::~Miner()
{
	if(graph!=0)
		delete graph;

	vect_map_destruct(frequentPatterns);
	vect_map_destruct(infrequentPatterns);
	vect_map_destruct(candidates);
}
