#pragma once

#include "MyEdge.h"
#include "MyGraph.h"
#include "core_file.h"

/**
 * represents a primary graph of a specific subgraph
 * primary graphs are used for join-based subgraph extension
 */
class PrimaryGraph {
private:
    MyGraph *graph;
    int src_node_id;//source from where the edge was deleted
    MyEdge *edge;//the edge that was deleted
    bool b;//false until a value is set for this primary graph

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

//	std::string getCanLabel() { return sig; }
    bool is_the_same_with(PrimaryGraph *other_pg);

    bool is_the_same_with(MyGraph *other_pg);

    bool is_set() { return b; }
};

/**
 * represents a frequent subgraph
 */
class Pattern {
private:
    int ID;
    int frequency; //initialized to -1 to indicate that it is not set
    MyGraph *graph = 0;
    bool graph_copied;
    std::vector<std::tr1::unordered_set<int> *> occurences;
    unsigned long predicted_time;
    int invalid_col = -1;
    int predicted_valids = -1;
    int subtasking_fixed = 1;
    int *temp_mni_table;
    bool result_exact;  //set to exact if the approximate frequency function can return exact result
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

    void combine(Pattern *other_p, int add_to_id = 0);//the two patterns should have a similar graph
    std::vector<std::tr1::unordered_set<int> *> *get_occurences() { return &occurences; }

    void invalidate_frequency() { frequency = -1; }

    int get_size();//get pattern size as the number of edges

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
