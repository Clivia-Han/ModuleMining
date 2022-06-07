/**
* 
MyMiner.h

 */

#ifndef MINERADV_H_
#define MINERADV_H_

#include "Miner.h"

class MyMiner : public Miner {
public:
    static long max_num_candids;
    map<int, set<int>> domains_solutions;
//	map<Pattern*, map<int, set<int>*>> frequent_patterns_domain;

    void start_mining(string file_name, int graph_type, int support, int given_type);

    void set_input_graph(GraphX *g) { this->graph = g; }

    GraphX *get_input_graph() { return graph; }

    void print_total_expected_time();

    void print_domains();

    void print_result(tr1::unordered_set<int> delete_pattern_id);

    char *
    pop_my_candidate(vector<CLMap *> &candidates, map<int, Pattern *> &currently_checking, int support,
                     double approximate);

    bool work_count(char *graph_str, int support, double approximate);

    int get_freq(Pattern *candidate, int support, double approximate);
};

#endif /* MINERADV_H_ */
