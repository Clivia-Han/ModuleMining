#pragma once

#include "core_file.h"

class NodeX {
private:
    int id;
    std::string label;
    std::tr1::unordered_map<int, void *> edges;
    std::tr1::unordered_map<int, void *> rev_edges;

public:
    NodeX(int id, std::string value);

    ~NodeX();

    void add_edge(NodeX *other_node, std::string edge_label, int graph_type);

    void remove_edge(NodeX *other_node, int graph_type);

    friend std::ostream &operator<<(std::ostream &os, const NodeX &n);

    int get_id() { return id; }

    std::string get_label() { return label; }

    std::tr1::unordered_map<int, void *>::iterator get_edges_iterator() { return edges.begin(); }

    std::tr1::unordered_map<int, void *>::iterator get_edges_end_iterator() { return edges.end(); }

    int get_edges_size() { return edges.size(); }

    void *get_edge_for_dest_node(int dest_node_id);

    bool is_it_connected_with_node_id(int node_id);

    bool is_it_connected_with_node_id(int node_id, std::string label);

    bool is_neighborhood_consistent(NodeX *node);
};