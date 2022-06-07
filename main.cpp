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

bool Settings::debug_msg;
int Settings::max_subgraph_size = -1;
int Settings::max_num_nodes = -1;
bool Settings::use_predicted_inv_column;

int Settings::min_number_of_samples;
int Settings::max_number_of_samples;

long Settings::postpone_nodes_after_iterations;

// user-given seed node
int Settings::given_type = -1;

int main() {
    //set application parameters
    GraMiCounter::num_side_effect_nodes = 0;
    Settings::debug_msg = true;
    Settings::postpone_nodes_after_iterations = 10000000;
    Settings::use_predicted_inv_column = true;

    //The miner routine
    Set_Iterator::st_SI = 0;

    //application default parameters
    string file_name = "../Datasets/graph_s";
//    string file_name = "../Datasets/patent_citations.lg";
    int graph_type = 0;//undirected graph
    int support = 2000;
//    Settings::given_type = 1;

    long long elapsed = 0;

    MyMiner *miner = new MyMiner();

    cout << "now start mining process" << endl;

    long long start_time = get_ms_of_day();

    miner->start_mining(file_name, graph_type, support, Settings::given_type);

    long long end_time = get_ms_of_day();
    elapsed = start_time - end_time;


    cout << "Mining took " << (elapsed / 1000) << " sec and " << (elapsed % 1000) << " ms" << endl;
    cout << "Finished!";
    return 0;
}
