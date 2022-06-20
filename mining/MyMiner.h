/**
* 
MyMiner.h

 */

#pragma once

#include "Miner.h"
#include "System.hpp"

class MyMiner : public Miner {
protected:
    std::set<int> id_set;
    std::map<int, int> id_map;
    GraphX *now_graph;

public:
    std::map<int, std::set<int>> domains_solutions;
//	std::map<Pattern*, std::map<int, std::set<int>*>> frequent_patterns_domain;

    void start_mining(std::string file_name, int graph_type, int support, std::string given_type);

    void start_mining_module(std::string file_name, int graph_type, int support, int seed_node_id);

    void start_mining_module(System &sym, int graph_type, int support, int seed_node_id, const std::string& path);

    void generate_now_graph(System &sym, int graph_type, int support, int seed_node_id);

    void set_input_graph(GraphX *g) { this->graph = g; }

    GraphX *get_input_graph() { return graph; }

    void print_total_expected_time();

    void print_domains();

    void print_result(std::tr1::unordered_set<int> delete_pattern_id);

    void print_frequent_module();

    void write_solutions(const std::string &path);

    char *
    pop_my_candidate(std::vector<CLMap *> &candidates, std::map<int, Pattern *> &currently_checking, int support,
                     double approximate);

    bool work_count(char *graph_str, int support, double approximate);

    int get_freq(Pattern *candidate, int support, double approximate);

    void DFS(int old_id, int new_id);
};
