/**
 * A representation of a graph, consisting of a set of nodes, and each node has a set of edges
 */
#pragma once

#include"MyNode.h"
#include "Settings.h"
#include "core_file.h"

class MyGraph {
private:
    int id;
    int edges_num;
    long freq = -1;     //frequency of the graph through mining process
    std::string sig;    //the signature of the graph which specifies the topological features
    std::tr1::unordered_map<int, MyNode *> nodes;
    std::tr1::unordered_map<std::string, std::set<int> *> nodes_by_label;   //get the nodes set according to the label
public:
    //initializers
    MyGraph();

    MyGraph(int);

    MyGraph(MyGraph *);

    ~MyGraph();

    void init(int);

    //add new node
    MyNode *add_node(int id, std::string label);

    //adding an edge
    void add_edge(int src_id, int dest_id, std::string edge_label);

    void add_edge(MyNode *src_node, MyNode *dest_node, std::string edge_label);

    void add_edge(int src_id, int dest_id, std::string edge_label, std::tr1::unordered_map<std::string, void *> &edge_to_freq);

    void add_edge(int src_id, int dest_id, std::string edge_label, std::tr1::unordered_map<std::string, void *> &edge_to_freq, std::map<std::string, std::map<int, std::set<int>>> &freq_edge_pairs);

    //remove an edge
    void remove_edge(int id1, int id2);

    void remove_edge_x(int id1, int id2);

    //remove a node and discard its incident edges
    void remove_node(int node_id);

    //iteratot functions
    std::tr1::unordered_map<int, MyNode *>::const_iterator get_nodes_iterator();

    std::tr1::unordered_map<int, MyNode *>::const_iterator get_nodes_end_iterator();

    int get_id() { return id; }

    std::string get_edge_label(int src_node, int dest_node);

    int get_nodes_num();

    MyNode *get_node_with_id(int node_id);

    std::set<int> *get_nodes_by_label(std::string label);

    friend std::ostream &operator<<(std::ostream &os, const MyGraph &g);

    int get_edges_num() { return edges_num; }

    //check if graph is connected
    bool is_connected();

    //this subgraph isomorphism code is correct
    void is_isomorphic(MyGraph *graph, std::vector<std::map<int, int> *> &results,
                       std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> &domains_values,
                       int restricted_domain_id = -1, int restricted_node_id = -1,
                       std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> *postponed_nodes = nullptr,
                       bool prune_by_domain_values = true, unsigned long max_iters = Settings::postpone_nodes_after_iterations);

    //graph isomorphism function
    bool same_with(MyGraph *other_g);

    //get a unique canonical label of this graph
    std::string get_sig();

    void reset_sig() { sig.clear(); }

    unsigned long num_iterations;

    void set_frequency(long freq) { this->freq = freq; }

    int get_frequency() { return this->freq; };

    std::tr1::unordered_map<std::string, std::set<int> *> get_nodes_by_label() {
        return nodes_by_label;
    }
};

std::ostream &operator<<(std::ostream &, const MyGraph &);

class SetIter {
public:
    SetIter() = default;

    ~SetIter() = default;

    std::set<int> *se{};//list of candidate nodes in the graph to match with
    std::set<int>::iterator it;//iterator over elements in se
    bool is_iter_end() { return it == se->end(); }
};
