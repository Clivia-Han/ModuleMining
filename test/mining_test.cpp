#include "core_file.h"
#include "MyMiner.h"
#include "Settings.h"
#include "mining_utils.h"

//set application default parameters

int Settings::support = 100;
int Settings::graph_type = 0;   //undirected graph
int Settings::given_flag = -1;  //user-given node flag
int Settings::max_edges_num = -1;
int Settings::max_nodes_num = -1;
bool Settings::debug_msg = false;
int Settings::given_seed_node_id = -1;  //user-given seed node id
std::string Settings::path = "../output_data";
bool Settings::use_predicted_inv_column = true;
bool Settings::throw_nodes_after_iterations = false;
long Settings::postpone_nodes_after_iterations = 10000000;
//std::string Settings::file_name = "../Datasets/now_graph.data";


int main() {
    std::cout << "***Project Kendinsky***\n";
    System sym;
    sym.init_unpre("../pre_data/aes_core.v.table");
    sym.store("../output_data");

    long long elapsed = 0;
    MyMiner *miner = new MyMiner();
    std::cout << "now start mining process" << std::endl;
    long long start_time = get_ms_of_day();
//    miner->start_mining(Settings::file_name, Settings::graph_type, Settings::support, Settings::given_type);
//    miner->start_mining_module(Settings::file_name, Settings::graph_type, Settings::support, Settings::given_seed_node_id);
    miner->start_mining_module(sym, Settings::graph_type, Settings::support, Settings::given_seed_node_id, Settings::path);
    long long end_time = get_ms_of_day();
    elapsed = start_time - end_time;

    std::cout << "Mining took " << (elapsed / 1000) << " sec and " << (elapsed % 1000) << " ms" << std::endl;
    std::cout << "Finished!";
    return 0;
}

