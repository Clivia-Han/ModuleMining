/**
 * Represent a graph node
 */

#include "MyEdge.h"
#include "MyNode.h"
#include "core_file.h"

MyNode::MyNode(int id, std::string label) {
    this->id = id;
    this->label = label;
}

MyNode::~MyNode() {
    for (auto ii : edges) {
        auto *edge = (MyEdge *) (ii.second);
        delete edge;
    }
    edges.clear();

    for (auto rev_edge : rev_edges) {
        auto *edge = (MyEdge *) (rev_edge.second);
        delete edge;
    }
    rev_edges.clear();
}

/**
 * Add an edge to the current node
 */
void MyNode::add_edge(MyNode *other_node, std::string edge_label) {
    // edges is all the edges connected with the current node
    if (edges.find(other_node->get_id()) != edges.end())
        return;
    auto *edge = new MyEdge(edge_label, other_node);

    edges[other_node->get_id()] = edge;
}

void MyNode::remove_edge(MyNode *other_node) {
    std::tr1::unordered_map<int, void *>::iterator iter = edges.find(other_node->get_id());
    if (iter != edges.end())
        delete (MyEdge*)(iter->second);
    edges.erase(other_node->get_id());
}

void *MyNode::get_edge_for_dest_node(int dest_node_id) {
    std::tr1::unordered_map<int, void *>::iterator temp = edges.find(dest_node_id);
    if (temp == edges.end())
        return nullptr;
    else
        return (*temp).second;
}

bool MyNode::is_connected_with(int node_id) {
    //check whether the node is connected with the current node by edges
    std::tr1::unordered_map<int, void *>::iterator iter = edges.find(node_id);
    if (iter == edges.end())
        return false;

    return true;
}

bool MyNode::is_connected_with(int node_id, std::string label) {
    //check whether the node is connected with the current node by edges
    std::tr1::unordered_map<int, void *>::iterator iter = edges.find(node_id);
    if (iter == edges.end())
        return false;

    //check edge label
    if (((MyEdge *) iter->second)->get_label() != label)
        return false;

    return true;
}
