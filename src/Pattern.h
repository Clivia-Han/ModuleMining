#ifndef PATTERN_H_
#define PATTERN_H_

#include <set>
#include<list>
#include<tr1/unordered_set>
#include "GraphX.h"
#include "EdgeX.h"

using namespace std;

/**
 * represents a primary graph of a specific subgraph
 * primary graphs are used for join-based subgraph extension
 */
class PrimaryGraph {
private:
    GraphX *graph;
    int src_node_id;//source from where the edge was deleted
    EdgeX *edge;//the edge that was deleted
    bool b;//false until a value is set for this primary graph

public:
    PrimaryGraph() { b = false; }

    ~PrimaryGraph() { delete graph; }

    void set_values(GraphX *graph, int srcNodeID, EdgeX *edge) {
        this->graph = graph;
        this->src_node_id = srcNodeID;
        this->edge = edge;
        b = true;
    }

    GraphX *get_graph() { return graph; }

    int get_src_node_id() { return src_node_id; }

    EdgeX *get_edge() { return edge; }

//	string getCanLabel() { return CL; }
    bool is_the_same_with(PrimaryGraph *other_pg);

    bool is_the_same_with(GraphX *other_pg);

    bool is_set() { return b; }
};

/**
 * represents a frequent subgraph
 */
class Pattern {
private:
    int ID;
    int frequency; //initialized to -1 to indicate that it is not set
    GraphX *graph = 0;
    bool graph_copied;
    vector<tr1::unordered_set<int> *> occurences;
    unsigned long predicted_time;
    int invalid_col = -1;
    int predicted_valids = -1;
    int predicted_freq = -1;

    int subtasking = 1;//how many tasks (workers) will be given to finish this pattern
    int subtasking_fixed = 1;

    void create_effe_graphs_list();

    void free_effe_graphs();

    int *temp_mni_table;
    int remaining_subtasks;
    bool result_exact;//set to exact if the approximate frequency function can return exact result
    int *postponedNodes_mniTable;
    unsigned long max_iters;

public:
    bool selected = false;
    int static st_counter;
    static int max_pattern_id;
    bool postpone_expensive_nodes;
    bool in_the_process_of_prediction = false;
    Pattern *predicted_pattern = 0;

    Pattern(GraphX *, bool copyGraph = true);

    Pattern(Pattern *);

    void init();

    ~Pattern();

    int get_frequency();

    map<int, set<int>> get_domain_values();

    int get_id();

    void set_frequency(int new_freq);

    void add_node(int node_id, int pattern_node_id);

    GraphX *get_graph() { return graph; }

    string to_string();

    void combine(Pattern *other_p, int add_to_id = 0);//the two patterns should have a similar graph
    vector<tr1::unordered_set<int> *> *get_occurences() { return &occurences; }

    void invalidate_frequency() { frequency = -1; }

    void extend(int src_id, int dest_id, double dest_label, double edge_label);//apply one edge extension
    bool do_i_need_to_count_in_this_partition(int);

    void set_partition_frequency(int freq, int part_id);

    int get_size();//get pattern size as the number of edges

    list<PrimaryGraph *> primary_graphs;

    set<std::pair<PrimaryGraph *, PrimaryGraph *> > get_joining_pg(Pattern *pattern);

    void generate_primary_graphs();

    bool has_unique_labels();

    void make_id_negative();

    void set_predicted_time(unsigned long pt) { this->predicted_time = pt; }

    void set_invalid_col(int inv_c, int p_valids);

//    void borrow_time_infor(Pattern *other_pattern, int n_workers);

    unsigned long get_predicted_time() { return this->predicted_time; }

    int get_invalid_col() { return invalid_col; }

    int get_predicted_valids() { return predicted_valids; }

    void set_subtasking(int st, int n_workers);

    int get_subtasking_value() { return this->subtasking; }

    int get_subtasking_value_fixed() { return this->subtasking_fixed; }

    void set_subtask_taken() { this->subtasking--; }

    bool subtasks_finished() { return subtasking == 0; }

    int sub_task_done(int *mni_table, int *postponed_mni_table, int support);

    void reset_mni();

    void set_result_exact() { result_exact = true; }

    bool is_result_exact() { return result_exact; }

    void set_predicted_freq(int pf) { this->predicted_freq = pf; }

    int get_predicted_freq() { return this->predicted_freq; }

    void set_max_iters(unsigned long mni) { max_iters = mni; }

    unsigned long get_max_iters() { return max_iters; }
};

#endif /* PATTERN_H_ */
