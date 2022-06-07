/**
 * Represent a graph node
 */

#include<iostream>
#include <stdlib.h>
#include<tr1/unordered_map>
#include "EdgeX.h"
#include "NodeX.h"

NodeX::NodeX(int id, double label) {
    this->id = id;
    this->label = label;
}

NodeX::~NodeX() {
    for (tr1::unordered_map<int, void *>::const_iterator ii = edges.begin(); ii != edges.end(); ++ii) {
        EdgeX *edge = (EdgeX *) ((*ii).second);
        delete edge;
    }
    edges.clear();

    for (tr1::unordered_map<int, void *>::const_iterator ii = rev_edges.begin(); ii != rev_edges.end(); ++ii) {
        EdgeX *edge = (EdgeX *) ((*ii).second);
        delete edge;
    }
    rev_edges.clear();
}

/**
 * Add an edge to this node
 * Parameters: the other node, and the edge label
 */
void NodeX::add_edge(NodeX *other_node, double edge_label, int graph_type) {
    // edges为当前结点关联的所有边
    if (edges.find(other_node->get_id()) != edges.end())
        return;
    EdgeX *edge = new EdgeX(edge_label, other_node);

    edges[other_node->get_id()] = edge;

    if (graph_type == 1) {
        EdgeX *edge = new EdgeX(edge_label, this);
        other_node->rev_edges[this->get_id()] = edge;
    }
}

void NodeX::remove_edge(NodeX *other_node, int graph_type) {
    tr1::unordered_map<int, void *>::iterator iter = edges.find(other_node->get_id());
    if (iter != edges.end())
        delete iter->second;
    edges.erase(other_node->get_id());

    if (graph_type == 1) {
        //do somethingh here
        tr1::unordered_map<int, void *>::iterator iter = other_node->rev_edges.find(this->get_id());
        if (iter != rev_edges.end())
            delete iter->second;
        rev_edges.erase(this->get_id());
    }
}

void *NodeX::get_edge_for_dest_node(int dest_node_id) {
    tr1::unordered_map<int, void *>::iterator temp = edges.find(dest_node_id);
    if (temp == edges.end())
        return NULL;
    else
        return (*temp).second;
}

bool NodeX::is_it_connected_with_node_id(int node_id) {
    //check node connectivity
    tr1::unordered_map<int, void *>::iterator iter = edges.find(node_id);
    if (iter == edges.end())
        return false;

    return true;
}

bool NodeX::is_it_connected_with_node_id(int node_id, double label) {
    //check node connectivity
    tr1::unordered_map<int, void *>::iterator iter = edges.find(node_id);
    if (iter == edges.end())
        return false;

    //check edge label
    if (((EdgeX *) iter->second)->get_label() != label)
        return false;

    return true;
}

/**
 * check whether the 'node' parameter in neighborhood consistent with 'this' node
 */
bool NodeX::is_neighborhood_consistent(NodeX *node) {
    // This method actually compares whether the total number of occurrences of each label (label distribution)
    // is equal in all neighbors of the current node and node node.
    tr1::unordered_map<double, int> labels;
    // populate labels of this node
    for (tr1::unordered_map<int, void *>::iterator iter = edges.begin(); iter != edges.end(); iter++) {
        // Count the labels of adjacent nodes,
        // and calculate the number of occurrences corresponding to each label
        double other_node_label = ((EdgeX *) iter->second)->get_label();
        tr1::unordered_map<double, int>::iterator temp_iter = labels.find(other_node_label);
        if (temp_iter == labels.end())
            labels.insert(std::pair<double, int>(other_node_label, 1));
        else {
            int current_count = temp_iter->second;
            labels.erase(other_node_label);
            labels.insert(std::pair<double, int>(other_node_label, current_count + 1));
        }
    }

    //check labels against this's labels
    for (tr1::unordered_map<int, void *>::iterator iter = node->get_edges_iterator(); iter !=
                                                                                      node->get_edges_end_iterator(); iter++) {
        //Traverse the adjacent nodes of the node
        double other_node_label = ((EdgeX *) iter->second)->get_label();
        //If the label of the adjacent node of the "node" does not appear in the label set of the current node's adjacent node, return false
        tr1::unordered_map<double, int>::iterator temp_iter = labels.find(other_node_label);
        if (temp_iter == labels.end())
            return false;
        //Otherwise, the corresponding value of the label in the labels dictionary is -1
        int current_count = temp_iter->second;
        labels.erase(other_node_label);
        if (current_count > 1)    //If currentCount<=1, then it is equivalent to directly remove otherNodeLabel from the labels dictionary
            labels.insert(std::pair<double, int>(other_node_label, current_count - 1));
    }

    return true;
}

ostream &operator<<(ostream &os, const NodeX &n) {
    // Output the information of node "n" and all its connected edges
    // (including edge labels and connection points) to os in the specified format
    os << n.id << '[' << n.label << "]" << endl;
    for (tr1::unordered_map<int, void *>::const_iterator ii = n.edges.begin(); ii != n.edges.end(); ++ii) {
        EdgeX *edge = (EdgeX *) ((*ii).second);
        os << "--" << edge->get_label() << "-->" << edge->get_other_node()->get_id() << endl;
    }
    return os;
}
