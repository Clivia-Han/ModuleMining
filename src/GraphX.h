/**
 * A representation of a graph, consisting of a set of nodes, and each node has a set of edges
 */
#ifndef GRAPHX_H_
#define GRAPHX_H_

#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include<set>
#include"NodeX.h"
#include "Settings.h"

using namespace std;

class GraphX {
private:
    int id;
    int type;//0-undirected, 1-directed, default value is 0. IMPORTANT: current algorithms do not support directed graphs
    tr1::unordered_map<int, NodeX *> nodes;
    int num_of_edges;
    tr1::unordered_map<double, set<int> *> nodes_by_label;
    string CL;//canonical label
    long freq = -1;//

public:
    //initializers
    GraphX();

    GraphX(int, int);

    void init(int, int);

    GraphX(GraphX *);

    ~GraphX();

    //load graph data
    bool parse_data(istream &data, tr1::unordered_map<string, void *> &edge_to_freq);

    bool parse_data(istream &data, tr1::unordered_map<string, void *> &edge_to_freq, map<string, map<int, set<int>>> &edge_pairs);

    bool load_from_file(string file_name, tr1::unordered_map<string, void *> &edge_to_freq);

    bool load_from_string(string data, tr1::unordered_map<string, void *> &edge_to_freq);

    bool load_from_file(string file_name, tr1::unordered_map<string, void *> &edge_to_freq, map<string, map<int, set<int>>> &edge_pairs);

    bool load_from_string(string data, tr1::unordered_map<string, void *> &edge_to_freq, map<string, map<int, set<int>>> &edge_pairs);

    //add new node
    NodeX *add_node(int id, double label);

    //adding an edge
    void add_edge(int src_id, int dest_id, double edge_label);

    void add_edge(NodeX *src_node, NodeX *dest_node, double edge_label);

    void add_edge(int src_id, int dest_id, double edge_label, tr1::unordered_map<string, void *> &edge_to_freq);

    void add_edge(int src_id, int dest_id, double edge_label, tr1::unordered_map<string, void *> &edge_to_freq, map<string, map<int, set<int>>> &freq_edge_pairs);

    //remove an edge
    void remove_edge(int id1, int id2);

    //remove a node and discard its incident edges
    void remove_node_ignore_edges(int node_id);

    //iteratot functions
    tr1::unordered_map<int, NodeX *>::const_iterator get_nodes_iterator();

    tr1::unordered_map<int, NodeX *>::const_iterator get_nodes_end_iterator();

    int get_id() { return id; }

    int get_type() { return type; }

    double get_edge_label(int src_node, int dest_node);

    int get_num_of_nodes();

    NodeX *get_node_with_id(int node_id);

    set<int> *get_nodes_by_label(double label);

    friend ostream &operator<<(ostream &os, const GraphX &g);

    int get_num_of_edges() { return num_of_edges; }

    //check if graph is connected
    bool is_connected();

    //this subgraph isomorphism code is correct
    void is_isomorphic(GraphX *graph, vector<map<int, int> *> &results,
                       tr1::unordered_map<int, tr1::unordered_set<int> *> &domains_values,
                       int restricted_domain_id = -1, int restricted_node_id = -1, bool prune_by_domain_values = true,
                       tr1::unordered_map<int, tr1::unordered_set<int> *> *postponed_nodes = 0,
                       unsigned long max_iters = Settings::postponeNodesAfterIterations);

    //graph isomorphism function
    bool is_the_same_with(GraphX *other_g);

    //get a unique canonical label of this graph
    string get_canonical_label();

    void reset_cl() { CL.clear(); }

    unsigned long num_iterations;

    bool can_satisfy_node_labels(GraphX *other_g);

    void set_frequency(long freq) { this->freq = freq; }
};

ostream &operator<<(ostream &, const GraphX &);

class Set_Iterator {
public:
    int static st_SI;

    Set_Iterator() { st_SI++; }

    ~Set_Iterator() { st_SI--; }

    set<int> *se;//list of candidate nodes in the graph to match with
    set<int>::iterator it;//iterator over elements in se
    bool is_iter_end() { return it == se->end(); }
};

#endif /* GRAPHX_H_ */
