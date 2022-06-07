#ifndef NODEX_H_
#define NODEX_H_

#include <map>
#include<vector>
#include<ostream>
#include <tr1/unordered_map>

using namespace std;

class NodeX {
private:
    int id;
    double label;
    tr1::unordered_map<int, void *> edges;
    tr1::unordered_map<int, void *> rev_edges;

public:
    NodeX(int id, double value);

    ~NodeX();

    void add_edge(NodeX *other_node, double edge_label, int graph_type);

    void remove_edge(NodeX *other_node, int graph_type);

    friend ostream &operator<<(ostream &os, const NodeX &n);

    int get_id() { return id; }

    double get_label() { return label; }

    tr1::unordered_map<int, void *>::iterator get_edges_iterator() { return edges.begin(); }

    tr1::unordered_map<int, void *>::iterator get_edges_end_iterator() { return edges.end(); }

    int get_edges_size() { return edges.size(); }

    void *get_edge_for_dest_node(int dest_node_id);

    bool is_it_connected_with_node_id(int node_id);

    bool is_it_connected_with_node_id(int node_id, double label);

    bool is_neighborhood_consistent(NodeX *node);
};

#endif /* NODEX_H_ */
