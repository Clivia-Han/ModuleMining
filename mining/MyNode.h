#pragma once

#include "core_file.h"

class MyNode {
private:
    int id;
    std::string label;
    std::tr1::unordered_map<int, void *> edges;
    std::tr1::unordered_map<int, void *> rev_edges;

public:
    MyNode(int id, std::string value);

    ~MyNode();

    int get_id() { return id; }

    std::string get_label() { return label; }

    int get_edges_size() { return edges.size(); }

    void add_edge(MyNode *other_node, std::string edge_label);

    void remove_edge(MyNode *other_node);

    bool is_connected_with(int node_id);

    bool is_connected_with(int node_id, std::string label);

    void *get_edge_for_dest_node(int dest_node_id);

    std::tr1::unordered_map<int, void *>::iterator get_edges_begin() { return edges.begin(); }

    std::tr1::unordered_map<int, void *>::iterator get_edges_end() { return edges.end(); }

    bool is_neighbor(MyNode* other_node);

    void remove_edge_by_id(int id);
};