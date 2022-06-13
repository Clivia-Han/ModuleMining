#include<iostream>
#include"MyMiner.h"
#include <cstdlib>
#include"Settings.h"
#include"utils.h"
#include "GraMiCounter.h"
#include "CLMap.h"
#include "Pattern.h"

#include<vector>
#include<map>

using namespace std;

int CL_Partition::st_counter;
int NodeWithInfo::st_counter;
int PartID_label::st_counter;
int Pattern::st_counter;
int Set_Iterator::st_SI;
bool Settings::debug_msg;
int Settings::max_subgraph_size = -1;
int Settings::max_num_nodes = -1;
bool Settings::use_predicted_inv_column;
long Settings::postponeNodesAfterIterations;
int Settings::min_number_of_samples;
int Settings::max_number_of_samples;
//user-given node flag
int Settings::given_flag = -1;
//user-given seed node id
int Settings::given_seed_node_id = -1;

long Settings::postpone_nodes_after_iterations;

int main() {
    //set application parameters
    GraMiCounter::num_side_effect_nodes = 0;
    Settings::debug_msg = false;
    Settings::postpone_nodes_after_iterations = 10000000;
    Settings::use_predicted_inv_column = true;
    //The miner routine
    Set_Iterator::st_SI = 0;

    //application default parameters
    string file_name = "../Datasets/graph.data";
//    string file_name = "../Datasets/patent_citations.lg";
    int graph_type = 0;//undirected graph
    int support = 100;
//    Settings::given_type = 1;

    long long elapsed = 0;

    MyMiner *miner = new MyMiner();

    cout << "now start mining process" << endl;

    long long start_time = get_ms_of_day();

//    miner->start_mining(file_name, graph_type, support, Settings::given_type);
    miner->start_mining_module(file_name, graph_type, support, Settings::given_seed_node_id);

    long long end_time = get_ms_of_day();
    elapsed = start_time - end_time;


    cout << "Mining took " << (elapsed / 1000) << " sec and " << (elapsed % 1000) << " ms" << endl;
    cout << "Finished!";
    return 0;
}

