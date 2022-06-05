#include<iostream>
#include"MyMiner.h"
#include <cstdlib>
#include"Settings.h"
#include"utils.h"
#include "GraMiCounter.h"
#include "CLMap.h"
#include "QueueSimulator.h"
#include "Pattern.h"

#include<vector>
#include<map>

using namespace std;

int CL_Partition::st_counter;
int NodeWithInfo::st_counter;
int PartID_label::st_counter;
int Pattern::st_counter;
int Set_Iterator::st_SI;
bool Settings::useSearchSpacePrediction;
bool Settings::divideBigTasks;
bool Settings::debugMSG;
int Settings::maxSubgraphSize = -1;
int Settings::maxNumNodes = -1;
int Settings::fixedNumSubtasks = -1;
double Settings::minImbalance;
bool Settings::usePredictedInvColumn;
bool Settings::smartBreak;

int Settings::minNumberOfSamples;
int Settings::maxNumberOfSamples;

long Settings::postponeNodesAfterIterations;

// user-given seed node
int Settings::givenType = -1;

int main( int argc, char *argv[] )
{
    //set application parameters
    GraMiCounter::numSideEffectNodes = 0;
    Settings::fixedNumSubtasks = -1;
    Settings::useSearchSpacePrediction = true;
//    Settings::divideBigTasks = true;
//    Settings::predictOnTheGo = true;
    Settings::debugMSG = true;
    Settings::postponeNodesAfterIterations = 10000000;
    Settings::showNumCandids = false;
    Settings::minImbalance = 15;
    Settings::usePredictedInvColumn = true;
    Settings::smartBreak = true;

    //The miner routine
    Set_Iterator::st_SI = 0;
    long long start = getmsofday();

    //application default parameters
    string fileName = "../Datasets/graph_s";
//    string fileName = "../Datasets/patent_citations.lg";
    int graphType = 0;//undirected graph
    int support = 2000;
//    Settings::givenType = 1;

    long long elapsed = 0;
    GraMiCounter::numByPassedNodes = 0;

    MyMiner* miner = new MyMiner();

    cout<<"Inexact search space building starts .................................."<<endl;

    long long startTime = getmsofday();

    miner->startMining(fileName, graphType, support, Settings::givenType);

    long long endTime = getmsofday();
    elapsed = startTime - endTime;


    cout<<"Mining took "<<(elapsed/1000)<<" sec and "<<(elapsed%1000)<<" ms"<<endl;
    cout<<"Finished!";
    return 0;
}
