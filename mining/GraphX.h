/**
 * A representation of a graph, consisting of a set of nodes, and each node has a set of edges
 */
#pragma once

#include"NodeX.h"
#include "Settings.h"
#include "core_file.h"

class GraphX {
private:
    int id;
    int type;//0-undirected, 1-directed, default value is 0. IMPORTANT: current algorithms do not support directed graphs
    std::tr1::unordered_map<int, NodeX *> nodes;
    int num_of_edges;
    std::tr1::unordered_map<std::string, std::set<int> *> nodes_by_label;
    std::string CL;//canonical label
    long freq = -1;
public:
    const std::tr1::unordered_map<std::string, std::set<int> *> &getNodesByLabel() const;

    //initializers
    GraphX();

    GraphX(int, int);

    void init(int, int);

    GraphX(GraphX *);

    ~GraphX();

    //load graph data
    bool parse_data(std::istream &data, std::tr1::unordered_map<std::string, void *> &edge_to_freq);

    bool parse_data(std::istream &data, std::tr1::unordered_map<std::string, void *> &edge_to_freq, std::map<std::string, std::map<int, std::set<int>>> &edge_pairs);

    bool load_from_file(std::string file_name, std::tr1::unordered_map<std::string, void *> &edge_to_freq);

    bool load_from_string(std::string data, std::tr1::unordered_map<std::string, void *> &edge_to_freq);

    bool load_from_file(std::string file_name, std::tr1::unordered_map<std::string, void *> &edge_to_freq, std::map<std::string, std::map<int, std::set<int>>> &edge_pairs);

    bool load_from_string(std::string data, std::tr1::unordered_map<std::string, void *> &edge_to_freq, std::map<std::string, std::map<int, std::set<int>>> &edge_pairs);

    //add new node
    NodeX *add_node(int id, std::string label);

    //adding an edge
    void add_edge(int src_id, int dest_id, std::string edge_label);

    void add_edge(NodeX *src_node, NodeX *dest_node, std::string edge_label);

    void add_edge(int src_id, int dest_id, std::string edge_label, std::tr1::unordered_map<std::string, void *> &edge_to_freq);

    void add_edge(int src_id, int dest_id, std::string edge_label, std::tr1::unordered_map<std::string, void *> &edge_to_freq, std::map<std::string, std::map<int, std::set<int>>> &freq_edge_pairs);

    //remove an edge
    void remove_edge(int id1, int id2);

    //remove a node and discard its incident edges
    void remove_node_ignore_edges(int node_id);

    //iteratot functions
    std::tr1::unordered_map<int, NodeX *>::const_iterator get_nodes_iterator();

    std::tr1::unordered_map<int, NodeX *>::const_iterator get_nodes_end_iterator();

    int get_id() { return id; }

    int get_type() { return type; }

    std::string get_edge_label(int src_node, int dest_node);

    int get_num_of_nodes();

    NodeX *get_node_with_id(int node_id);

    std::set<int> *get_nodes_by_label(std::string label);

    friend std::ostream &operator<<(std::ostream &os, const GraphX &g);

    int get_num_of_edges() { return num_of_edges; }

    //check if graph is connected
    bool is_connected();

    //this subgraph isomorphism code is correct
    void is_isomorphic(GraphX *graph, std::vector<std::map<int, int> *> &results,
                       std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> &domains_values,
                       int restricted_domain_id = -1, int restricted_node_id = -1,
                       std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> *postponed_nodes = nullptr,
                       bool prune_by_domain_values = true, unsigned long max_iters = Settings::postpone_nodes_after_iterations);

    //graph isomorphism function
    bool is_the_same_with(GraphX *other_g);

    //get a unique canonical label of this graph
    std::string get_canonical_label();

    void reset_cl() { CL.clear(); }

    unsigned long num_iterations;

    bool can_satisfy_node_labels(GraphX *other_g);

    void set_frequency(long freq) { this->freq = freq; }

    int get_frequency() { return this->freq; };

    std::tr1::unordered_map<std::string, std::set<int> *> get_nodes_by_label() {
        return nodes_by_label;
    }
};

std::ostream &operator<<(std::ostream &, const GraphX &);

class Set_Iterator {
public:
    Set_Iterator() = default;

    ~Set_Iterator() = default;

    std::set<int> *se{};//list of candidate nodes in the graph to match with
    std::set<int>::iterator it;//iterator over elements in se
    bool is_iter_end() { return it == se->end(); }
};
