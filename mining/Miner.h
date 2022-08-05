/**
* 
Miner.h

 */

#pragma once
#include "SigMap.h"
#include "Pattern.h"
#include "System.hpp"

class Miner {

private:
    SigMap processed;

protected:
    std::map<int, std::map<int, std::set<int>>> frequent_patterns_domain;
    std::vector<Pattern *> frequent_pattern_vec;

    MyGraph *graph = 0;
    int support{};
    SigMap frequent_edges;
    std::map<std::string, std::map<int, std::set<int>>> freq_pattern_pairs;
    std::map<int, std::set<int>> freq_edge_pairs;
    std::set<std::string> sig_set;

    //frequent patterns sorted in a vector, nth element in the vector is hash map of all patterns having n edges
    //each hashmap is keyed by the canonical form of the value graph (pattern)
    std::vector<SigMap *> frequent_patterns;
    std::vector<SigMap *> infrequent_patterns;
    std::vector<SigMap *> candidates;
    std::map<int, Pattern *> currently_checking;

    std::set<int> id_set;
    std::map<int, int> id_map;
    MyGraph *now_graph{};

public:
    Miner();

    ~Miner();

    std::map<int, std::set<int>> domains_solutions;

    void start_mining_module(System &sym, int support, int seed_node_id);

    void generate_now_graph(System &sym, int support, int seed_node_id);

    void set_input_graph(MyGraph *g) { this->graph = g; }

    MyGraph *get_input_graph() { return graph; }

    void print_domains();

    void print_frequent_module();

    void write_solutions(const std::string &path);

    void DFS(int old_id, int new_id);

    void store_frequent_instance(const std::string &path);

    void domain_dfs(int node_id, int graph_id, std::ofstream& out);
};
