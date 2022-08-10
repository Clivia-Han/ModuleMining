#pragma once

#include "MyEdge.h"
#include "MyGraph.h"
#include "core_file.h"

class PrimaryGraph {
private:
    MyGraph *graph;
    int src_node_id;
    MyEdge *edge;
    bool b;

public:
    PrimaryGraph() { b = false; }

    ~PrimaryGraph() { delete graph; }

    void set_values(MyGraph *graph, int srcNodeID, MyEdge *edge) {
        this->graph = graph;
        this->src_node_id = srcNodeID;
        this->edge = edge;
        b = true;
    }

    MyGraph *get_graph() { return graph; }

    int get_src_node_id() { return src_node_id; }

    MyEdge *get_edge() { return edge; }

    bool same_with(PrimaryGraph *other_pg);

    bool same_with(MyGraph *other_pg);

    bool is_set() { return b; }
};


class Pattern {
private:
    int ID;
    int frequency;
    MyGraph *graph = 0;
    bool graph_copied;
    std::vector<std::tr1::unordered_set<int> *> occurences;
    unsigned long predicted_time;
    int invalid_col = -1;
    int predicted_valids = -1;
    int subtasking_fixed = 1;
    int *temp_mni_table;
    bool result_exact;
    int *postponedNodes_mniTable;
    unsigned long max_iters;

public:
    static int max_pattern_id;
    bool postpone_expensive_nodes;

    Pattern(MyGraph *, bool copyGraph = true);

    Pattern(Pattern *);

    void init();

    ~Pattern();

    int get_frequency();

    std::map<int, std::set<int>> get_domain_values();

    int get_id();

    void set_frequency(int new_freq);

    void add_node(int node_id, int pattern_node_id);

    MyGraph *get_graph() { return graph; }

    std::string to_string();

    void combine(Pattern *other_p, int add_to_id = 0);

    std::vector<std::tr1::unordered_set<int> *> *get_occurences() { return &occurences; }

    void invalidate_frequency() { frequency = -1; }

    int get_size();

    std::list<PrimaryGraph *> primary_graphs;

    std::set<std::pair<PrimaryGraph *, PrimaryGraph *> > get_joining_pg(Pattern *pattern);

    bool has_unique_labels();

    void set_invalid_col(int inv_c, int p_valids);

    unsigned long get_predicted_time() { return this->predicted_time; }

    int get_invalid_col() { return invalid_col; }

    int get_predicted_valids() { return predicted_valids; }

    bool is_result_exact() { return result_exact; }

    void set_max_iters(unsigned long mni) { max_iters = mni; }

    unsigned long get_max_iters() { return max_iters; }
};
