/**
 * A representation of a graph, consisting of a set of nodes, and each node has a set of edges
 */

#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include<iostream>
#include<fstream>
#include<sstream>
#include <stdlib.h>
#include <unistd.h>
#include <limits>
#include "GraphX.h"
#include "EdgeX.h"
#include "utils.h"
#include "Pattern.h"
#include "GraMiCounter.h"
#include "Settings.h"
#include <iomanip>

bool check_current_status(GraphX *graph1, GraphX *graph2, vector<Set_Iterator *> &current_s, int *nodes_order,
                          tr1::unordered_map<int, int> &selected_query_map) {
    NodeX *data_graph_node = graph2->get_node_with_id(*(current_s[current_s.size() - 1]->it));
    NodeX *query_graph_node = graph1->get_node_with_id(nodes_order[current_s.size() - 1]);

    for (tr1::unordered_map<int, void *>::iterator iter = query_graph_node->get_edges_iterator(); iter !=
                                                                                                  query_graph_node->get_edges_end_iterator(); ++iter) {
        int other_node_id = iter->first;
        EdgeX *edge = (EdgeX *) (iter->second);

        if (selected_query_map.find(other_node_id) == selected_query_map.end())
            continue;

        //get the data node that the current data node should be connected with
        // selected_query_map.find(other_node_id)->second 是 query图中otherNodeID对应的结点的order
        // 下面找到data图中对应的结点ID
        int data_node_id = *(current_s[selected_query_map.find(other_node_id)->second]->it);

        //check the current graph edges, whether it has a connection to data_node_id or not
        if (!data_graph_node->is_it_connected_with_node_id(data_node_id, edge->get_label())) {
            //cout<<"false"<<endl;
            return false;
        }
    }

    return true;
}

void insert_into_edge_freq(NodeX *src, int dest_node_id, double dest_node_label, double edge_label,
                           tr1::unordered_map<string, void *> &edge_to_freq,
                           map<string, map<int, set<int>>> &freq_edge_pairs, bool add_src_only) {
    string key;
    if (src->get_label() > dest_node_label) {
        stringstream ss;
        if (src->get_label() == dest_node_label)
            ss << src->get_label() << dest_node_label << "," << edge_label << ",";
        else
            ss << src->get_label() << "," << dest_node_label << "," << edge_label << ",";
        key = ss.str();
    } else {
        stringstream ss;
        if (dest_node_label == src->get_label())
            ss << dest_node_label << src->get_label() << "," << edge_label << ",";
        else
            ss << dest_node_label << "," << src->get_label() << "," << edge_label << ",";
        key = ss.str();
    }

    tr1::unordered_map<string, void *>::iterator iter = edge_to_freq.find(key);
    Pattern *pattern;
    if (iter == edge_to_freq.end()) {
        GraphX *graph = new GraphX();
        NodeX *node1 = graph->add_node(0, src->get_label());
        NodeX *node2 = graph->add_node(1, dest_node_label);
        graph->add_edge(node1, node2, edge_label);
        pattern = new Pattern(graph);
        delete graph;

        edge_to_freq[key] = pattern;

        set<int> st1 = set<int>();
        set<int> st2 = set<int>();
        st1.insert(src->get_id());
        st2.insert(dest_node_id);
        map<int, set<int>> mp = map<int, set<int>>();
        mp[dest_node_id] = st1;
        mp[src->get_id()] = st2;
        freq_edge_pairs[key] = mp;
    } else {
        if (freq_edge_pairs[key].find(src->get_id()) == freq_edge_pairs[key].end()) {
            freq_edge_pairs[key][src->get_id()] = set<int>();
            freq_edge_pairs[key][src->get_id()].insert(dest_node_id);
        } else {
            freq_edge_pairs[key][src->get_id()].insert(dest_node_id);
        }
        if (freq_edge_pairs[key].find(dest_node_id) == freq_edge_pairs[key].end()) {
            freq_edge_pairs[key][dest_node_id] = set<int>();
            freq_edge_pairs[key][dest_node_id].insert(src->get_id());
        } else {
            freq_edge_pairs[key][dest_node_id].insert(src->get_id());
        }
        pattern = (Pattern *) ((*iter).second);
    }

    if (pattern->get_graph()->get_node_with_id(0)->get_label() == src->get_label()) {
        pattern->add_node(src->get_id(), 0);
        if (!add_src_only) pattern->add_node(dest_node_id, 1);
    } else {
        pattern->add_node(src->get_id(), 1);
        if (!add_src_only) pattern->add_node(dest_node_id, 0);
    }
}

void insert_into_edge_freq(NodeX *src, int dest_node_id, double dest_node_label, double edge_label,
                           tr1::unordered_map<string, void *> &edge_to_freq, bool add_src_only) {
    string key;
    if (src->get_label() > dest_node_label) {
        stringstream ss;
        if (src->get_label() == dest_node_label)
            ss << src->get_label() << dest_node_label << "," << edge_label << ",";
        else
            ss << src->get_label() << "," << dest_node_label << "," << edge_label << ",";
        key = ss.str();
    } else {
        stringstream ss;
        if (dest_node_label == src->get_label())
            ss << dest_node_label << src->get_label() << "," << edge_label << ",";
        else
            ss << dest_node_label << "," << src->get_label() << "," << edge_label << ",";
        key = ss.str();
    }

    tr1::unordered_map<string, void *>::iterator iter = edge_to_freq.find(key);
    Pattern *pattern;
    if (iter == edge_to_freq.end()) {
        GraphX *graph = new GraphX();
        NodeX *node1 = graph->add_node(0, src->get_label());
        NodeX *node2 = graph->add_node(1, dest_node_label);
        graph->add_edge(node1, node2, edge_label);
        pattern = new Pattern(graph);
        delete graph;

        edge_to_freq[key] = pattern;
    } else {
        pattern = (Pattern *) ((*iter).second);
    }

    if (pattern->get_graph()->get_node_with_id(0)->get_label() == src->get_label()) {
        pattern->add_node(src->get_id(), 0);
        if (!add_src_only) pattern->add_node(dest_node_id, 1);
    } else {
        pattern->add_node(src->get_id(), 1);
        if (!add_src_only) pattern->add_node(dest_node_id, 0);
    }
}

/**
 * Constructor
 */
GraphX::GraphX() {
    init(0, 0);
}

GraphX::GraphX(int id, int type) {
    init(id, type);
}

void GraphX::init(int id, int type) {
    this->id = id;
    this->type = type;
    if (type == 1) {
        cout << "IMPORTANT! current set of algorithms are not tested on directed graphs!";
        exit(0);
    }
    num_of_edges = 0;
    this->freq = -1;
}

GraphX::GraphX(GraphX *graph) {
    this->id = graph->get_id();
    this->type = graph->get_type();
    num_of_edges = graph->get_num_of_edges();

    //copy nodes
    for (tr1::unordered_map<int, NodeX *>::const_iterator iter = graph->get_nodes_iterator(); iter !=
                                                                                              graph->get_nodes_end_iterator(); ++iter) {
        NodeX *oldNode = iter->second;
        this->add_node(oldNode->get_id(), oldNode->get_label());
    }

    //copy edges
    for (tr1::unordered_map<int, NodeX *>::const_iterator iter = graph->get_nodes_iterator(); iter !=
                                                                                              graph->get_nodes_end_iterator(); ++iter) {
        NodeX *old_node = iter->second;
        NodeX *node = nodes.find(old_node->get_id())->second;

        for (tr1::unordered_map<int, void *>::iterator iter2 = old_node->get_edges_iterator(); iter2 !=
                                                                                               old_node->get_edges_end_iterator(); ++iter2) {
            EdgeX *edge = (EdgeX *) (iter2->second);
            node->add_edge(nodes.find(edge->get_other_node()->get_id())->second, edge->get_label(), this->type);
        }
    }
}

/**
 * Add a node to the graph
 * Parameters are: node id, and node label
 */
NodeX *GraphX::add_node(int id, double label) {
    CL.clear();

    tr1::unordered_map<int, NodeX *>::iterator temp = nodes.find(id);
    if (temp != nodes.end())
        return temp->second;

    NodeX *node = new NodeX(id, label);

    nodes.insert(std::pair<int, NodeX *>(id, node));

    //add to the 'nodes by label' map
    tr1::unordered_map<double, set<int> *>::iterator iter = nodes_by_label.find(label);
    if (iter == nodes_by_label.end()) {
        nodes_by_label.insert(std::pair<double, set<int> *>(label, new set<int>()));
        iter = nodes_by_label.find(label);
    }
    iter->second->insert(id);

    return node;
}

bool GraphX::can_satisfy_node_labels(GraphX *other_g) {
    for (tr1::unordered_map<double, set<int> *>::iterator iter = other_g->nodes_by_label.begin();
         iter != other_g->nodes_by_label.end(); iter++) {
        double label = (*iter).first;
        int count = (*iter).second->size();
        if (count == 0) continue;

        tr1::unordered_map<double, set<int> *>::iterator this_iter = nodes_by_label.find(label);
        if (this_iter == nodes_by_label.end())
            return false;
        int this_count = (*iter).second->size();
        if (this_count < count)
            return false;
    }

    return true;
}

/**
 * Add an edge between two nodes in the graph, the nodes must exist before adding the edge
 * Parameters: the source node ID, the destination node ID, and the edge label
 * For undirected graphs, one more edge will be added in the reverse direction
 */
void GraphX::add_edge(NodeX *src_node, NodeX *dest_node, double edge_label) {
    CL.clear();

    src_node->add_edge(dest_node, edge_label, this->type);

    if (this->type == 0) {
        dest_node->add_edge(src_node, edge_label, this->type);
    }

    num_of_edges++;
}

/**
 * Add an edge to this graph given source node, destination node, and an edge label
 */
void GraphX::add_edge(int src_id, int dest_id, double edge_label) {
    CL.clear();

    NodeX *src_node;
    NodeX *dest_node;

    tr1::unordered_map<int, NodeX *>::iterator iter = nodes.find(src_id);
    if (iter != nodes.end())
        src_node = nodes[src_id];
    else
        return;

    iter = nodes.find(dest_id);
    if (iter != nodes.end())
        dest_node = nodes[dest_id];
    else
        return;

    src_node->add_edge(dest_node, edge_label, this->type);

    if (this->type == 0) {
        dest_node->add_edge(src_node, edge_label, this->type);
    }

    num_of_edges++;

    return;
}

/**
 * remove an edge by its incident node ids
 */
void GraphX::remove_edge(int id1, int id2) {
    CL.clear();

    nodes[id1]->remove_edge(nodes[id2], this->type);
    if (nodes[id1]->get_edges_size() == 0)
        remove_node_ignore_edges(id1);

    if (this->type == 0) {
        nodes[id2]->remove_edge(nodes[id1], this->type);
        if (nodes[id2]->get_edges_size() == 0)
            remove_node_ignore_edges(id2);
    }

    num_of_edges--;
}

/**
 * remove a node from the graph, ignoring edges (as a prepost all edges connecting to this node should be already removed)
 */
void GraphX::remove_node_ignore_edges(int node_id) {
    CL.clear();

    nodes_by_label.find(get_node_with_id(node_id)->get_label())->second->erase(node_id);
    delete nodes.find(node_id)->second;
    nodes.erase(node_id);
}

void GraphX::add_edge(int src_id, int dest_id, double edge_label, tr1::unordered_map<string, void *> &edge_to_freq, map<string, map<int, set<int>>> &freq_edge_pairs) {
    if (CL.length() > 0)
        CL.clear();

    NodeX *src_node;
    NodeX *dest_node;

    tr1::unordered_map<int, NodeX *>::iterator iter = nodes.find(src_id);
    if (iter != nodes.end())
        src_node = iter->second;
    else
        return;

    iter = nodes.find(dest_id);
    if (iter != nodes.end())
        dest_node = iter->second;
    else
        return;

    this->add_edge(src_node, dest_node, edge_label);

    insert_into_edge_freq(src_node, dest_id, dest_node->get_label(), edge_label, edge_to_freq, freq_edge_pairs, false);

    if (this->type == 0) {
        insert_into_edge_freq(dest_node, src_id, src_node->get_label(), edge_label, edge_to_freq, freq_edge_pairs, false);
    }
}

void GraphX::add_edge(int src_id, int dest_id, double edge_label, tr1::unordered_map<string, void *> &edge_to_freq) {
    if (CL.length() > 0)
        CL.clear();

    NodeX *src_node;
    NodeX *dest_node;

    tr1::unordered_map<int, NodeX *>::iterator iter = nodes.find(src_id);
    if (iter != nodes.end())
        src_node = iter->second;
    else
        return;

    iter = nodes.find(dest_id);
    if (iter != nodes.end())
        dest_node = iter->second;
    else
        return;

    this->add_edge(src_node, dest_node, edge_label);

    insert_into_edge_freq(src_node, dest_id, dest_node->get_label(), edge_label, edge_to_freq, false);

    if (this->type == 0) {
        insert_into_edge_freq(dest_node, src_id, src_node->get_label(), edge_label, edge_to_freq, false);
    }
}

/**
 * Load a graph file that has .lg format
 * return true if loading is done correctly, otherwise false
 */
bool GraphX::load_from_file(string file_name, tr1::unordered_map<string, void *> &edge_to_freq) {
    CL.clear();

    cout << "Loading graph from file: " << file_name << endl;
    ifstream file(file_name.c_str(), ios::in);
    if (!file) {
        cout << "While opening a file an error is encountered" << endl;
        return false;
    }

    if (!parse_data(file, edge_to_freq))
        return false;

    file.close();

    return true;
}

/**
 * load a graph from the given string. string should follow the .lg format
 */
bool GraphX::load_from_string(string data, tr1::unordered_map<string, void *> &edge_to_freq) {
    CL.clear();

    istringstream str(data);

    bool b = parse_data(str, edge_to_freq);

    //destruct data in the 'edgeToFreq' structure
    for (tr1::unordered_map<string, void *>::iterator iter = edge_to_freq.begin(); iter != edge_to_freq.end(); iter++) {
        delete ((Pattern *) iter->second);
    }
    edge_to_freq.clear();

    if (!b)
        return false;

    return true;
}

/**
 * Load a graph file that has .lg format
 * return true if loading is done correctly, otherwise false
 */
bool GraphX::load_from_file(string file_name, tr1::unordered_map<string, void *> &edge_to_freq, map<string, map<int, set<int>>> &edge_pairs) {
    CL.clear();

    cout << "Loading graph from file: " << file_name << endl;
    ifstream file(file_name.c_str(), ios::in);
    if (!file) {
        cout << "While opening a file an error is encountered" << endl;
        return false;
    }

    if (!parse_data(file, edge_to_freq, edge_pairs))
        return false;

    file.close();

    return true;
}

/**
 * load a graph from the given string. string should follow the .lg format
 */
bool GraphX::load_from_string(string data, tr1::unordered_map<string, void *> &edge_to_freq, map<string, map<int, set<int>>> &edge_pairs) {
    CL.clear();

    istringstream str(data);

    bool b = parse_data(str, edge_to_freq, edge_pairs);

    //destruct data in the 'edgeToFreq' structure
    for (tr1::unordered_map<string, void *>::iterator iter = edge_to_freq.begin(); iter != edge_to_freq.end(); iter++) {
        delete ((Pattern *) iter->second);
    }
    edge_to_freq.clear();

    if (!b)
        return false;

    return true;
}

/**
 * the graph loader parser
 */
bool GraphX::parse_data(istream &data, tr1::unordered_map<string, void *> &edge_to_freq, map<string, map<int, set<int>>> &edge_pairs) {
    //read the first line
    char temp_ch;
    data >> temp_ch;
    data >> temp_ch;
    data >> temp_ch;

    int num_edges_loaded = 0;

    bool first_edge_met = false;
    while (!data.eof()) {
        char ch;
        ch = '\0';
        data >> ch;

        //to add nodes
        if (ch == 'v') {
            int id;
            double label;
            data >> id;
            data >> label;

            this->add_node(id, label);
        } else if (ch == 'e')//to add edges
        {
            if (!first_edge_met) {
                first_edge_met = true;

                if (freq > -1) {
                    for (tr1::unordered_map<double, set<int> *>::iterator iter = this->nodes_by_label.begin();
                         iter != nodes_by_label.end(); iter++) {
                        if (iter->second->size() < freq) {
                            //如果带某标签的节点在图中出现的次数小于阈值，也就意味着与带有该标签的节点相连的所有边都是不频繁的
                            tr1::unordered_map<double, set<int> *>::iterator iter2 = this->nodes_by_label.find(
                                    iter->first);
                            if (iter2 == this->nodes_by_label.end()) {
                                cout << "Error: isufnm44" << endl;
                                exit(0);
                            }
                            //遍历所有带该标签的节点，iter2为其中的节点，忽略此节点与和此节点相连的边
                            set<int> *nodes_to_remove = iter2->second;
                            for (set<int>::iterator iter1 = nodes_to_remove->begin();
                                 iter1 != nodes_to_remove->end();) {
                                int nodeID = *iter1;
                                iter1++;
                                this->remove_node_ignore_edges(nodeID);
                            }
                        }
                    }
                }
            }

            int id1;
            int id2;
            double label;
            data >> id1;
            data >> id2;
            data >> label;
            this->add_edge(id1, id2, label, edge_to_freq, edge_pairs);
            num_edges_loaded++;
        }
    }

    return true;
}

/**
 * the graph loader parser
 */
bool GraphX::parse_data(istream &data, tr1::unordered_map<string, void *> &edge_to_freq) {
    //read the first line
    char temp_ch;
    data >> temp_ch;
    data >> temp_ch;
    data >> temp_ch;

    int num_edges_loaded = 0;

    bool first_edge_met = false;
    while (!data.eof()) {
        char ch;
        ch = '\0';
        data >> ch;

        //to add nodes
        if (ch == 'v') {
            int id;
            double label;
            data >> id;
            data >> label;

            this->add_node(id, label);
        } else if (ch == 'e')//to add edges
        {
            if (!first_edge_met) {
                first_edge_met = true;

                if (freq > -1) {
                    for (tr1::unordered_map<double, set<int> *>::iterator iter = this->nodes_by_label.begin();
                         iter != nodes_by_label.end(); iter++) {
                        if (iter->second->size() < freq) {
                            //如果带某标签的节点在图中出现的次数小于阈值，也就意味着与带有该标签的节点相连的所有边都是不频繁的
                            tr1::unordered_map<double, set<int> *>::iterator iter2 = this->nodes_by_label.find(
                                    iter->first);
                            if (iter2 == this->nodes_by_label.end()) {
                                cout << "Error: isufnm44" << endl;
                                exit(0);
                            }
                            //遍历所有带该标签的节点，iter2为其中的节点，忽略此节点与和此节点相连的边
                            set<int> *nodes_to_remove = iter2->second;
                            for (set<int>::iterator iter1 = nodes_to_remove->begin();
                                 iter1 != nodes_to_remove->end();) {
                                int nodeID = *iter1;
                                iter1++;
                                this->remove_node_ignore_edges(nodeID);
                            }
                        }
                    }
                }
            }

            int id1;
            int id2;
            double label;
            data >> id1;
            data >> id2;
            data >> label;
            this->add_edge(id1, id2, label, edge_to_freq);
            num_edges_loaded++;
        }
    }

    return true;
}

NodeX *GraphX::get_node_with_id(int node_id) {
    tr1::unordered_map<int, NodeX *>::iterator iter = nodes.find(node_id);
    if (iter == nodes.end())
        return NULL;
    else
        return iter->second;
}

set<int> *GraphX::get_nodes_by_label(double label) {
    tr1::unordered_map<double, set<int> *>::iterator iter = nodes_by_label.find(label);
    if (iter == nodes_by_label.end())
        return NULL;
    return iter->second;
}

tr1::unordered_map<int, NodeX *>::const_iterator GraphX::get_nodes_iterator() {
    return nodes.begin();
}

tr1::unordered_map<int, NodeX *>::const_iterator GraphX::get_nodes_end_iterator() {
    return nodes.end();
}

/**
 * get edge label from the scr node to the destination node
 * if either the src node is not found, or the detination node is not found from the src node, then return 0.0001
 */
double GraphX::get_edge_label(int src_node_id, int dest_node_id) {
    NodeX *srcNode;
    tr1::unordered_map<int, NodeX *>::iterator temp = nodes.find(src_node_id);
    if (temp == nodes.end())
        return 0.0001;
    srcNode = (*temp).second;

    EdgeX *edge = (EdgeX *) srcNode->get_edge_for_dest_node(dest_node_id);
    if (edge == NULL)
        return 0.0001;

    return edge->get_label();
}

int GraphX::get_num_of_nodes() {
    return this->nodes.size();
}

/**
 * return treu if the graph is connected
 */
bool GraphX::is_connected() {
    if (nodes.size() < 2)
        return true;

    //start from any node
    NodeX *node = nodes.begin()->second;
    map<int, NodeX *> visited;
    map<int, NodeX *> to_visit;

    to_visit.insert(std::pair<int, NodeX *>(node->get_id(), node));
    while (to_visit.size() > 0) {
        //1- pop a node for the to be visited list, 2- remove it from to_visit, and 3- add it to the visited list
        node = to_visit.begin()->second;//1
        to_visit.erase(to_visit.begin());//2
        visited.insert(std::pair<int, NodeX *>(node->get_id(), node));//3
        //add its neighbors
        for (tr1::unordered_map<int, void *>::iterator iter = node->get_edges_iterator(); iter !=
                                                                                          node->get_edges_end_iterator(); ++iter) {
            int id = iter->first;
            if (visited.find(id) != visited.end())
                continue;
            EdgeX *edge = (EdgeX *) iter->second;
            to_visit.insert(std::pair<int, NodeX *>(id, edge->get_other_node()));
        }
    }

    if (visited.size() == nodes.size())
        return true;
    else
        return false;

}

/**
 * return true if the two graphs are the same
 */
bool GraphX::is_the_same_with(GraphX *other_g) {
    if (this->get_num_of_nodes() != other_g->get_num_of_nodes() ||
        this->get_num_of_edges() != other_g->get_num_of_edges()) {
        return false;
    }

    //compare using CL
    string this_cl = this->get_canonical_label();
    string other_cl = other_g->get_canonical_label();
    if (this_cl.compare(other_cl) != 0)
        return false;
    else {
        bool a = false;
        if (this_cl.at(0) == 'X') a = true;

        if (!a)
            return true;
    }

    vector<map<int, int> *> result;
    tr1::unordered_map<int, tr1::unordered_set<int> *> domains_values;

    this->is_isomorphic(other_g, result, domains_values, -1, -1, false);

    bool b;

    if (result.size() > 0) {
        b = true;
    } else {
        b = false;
    }

    for (vector<map<int, int> *>::iterator iter1 = result.begin(); iter1 != result.end(); iter1++) {
        delete (*iter1);
    }
    result.clear();

    return b;
}

/**
 * check whether two graphs are isomorphic or not
 * if isomorphic, then 'results' will have mapping between nodes from this graph to nodes in the given graph
 * 'this' is the query graph, while graph i the data graph, this means I can query this (smaller) in the data (Bigger)
 *  [*] this function does not consider edge label! FIX IT!!!!
 */
void GraphX::is_isomorphic(GraphX *graph, vector<map<int, int> *> &results,
                           tr1::unordered_map<int, tr1::unordered_set<int> *> &domains_values, int restricted_domain_id,
                           int restricted_node_id, bool prune_by_domain_values,
                           tr1::unordered_map<int, tr1::unordered_set<int> *> *postponed_nodes,
                           unsigned long max_iters) {
    //a variable for counting
    num_iterations = 0;

    //populate the order in which query graph will be traversed
    tr1::unordered_set<int> checked;
    int nodes_order[get_num_of_nodes()];//the result of this part

    //get the nodes order
    vector<int> to_check;

    //check for the restricted node, if it exists let it be the first to check
    if (restricted_domain_id == -1)
        to_check.push_back(nodes.begin()->second->get_id());
    else
        to_check.push_back(restricted_domain_id);

    int count = 0;
    while (to_check.size() > 0) {
        int current = to_check.front();
        to_check.erase(to_check.begin());
        if (checked.find(current) != checked.end())
            continue;
        nodes_order[count] = current;
        checked.insert(current);
        count++;
        NodeX *curr_node = nodes[current];

        int start = to_check.size();
        // 将currNode的邻接结点依次放入toCheck列表
        for (tr1::unordered_map<int, void *>::iterator iter = curr_node->get_edges_iterator(); iter !=
                                                                                               curr_node->get_edges_end_iterator(); ++iter) {
            int other_id = iter->first;
            if (checked.find(other_id) == checked.end()) {
                //put them in order based on the degree
                vector<int>::iterator iter1 = to_check.begin();
                for (int i = 0; i < start; i++)
                    iter1++;
                for (; iter1 != to_check.end(); ++iter1) {
                    // 按度递增的顺序排序
                    if (nodes[other_id]->get_edges_size() < nodes[(*iter1)]->get_edges_size())
                        break;
                }
                to_check.insert(iter1, other_id);
            }
        }
    }

    vector<Set_Iterator *> selected;
    tr1::unordered_set<int> selected_data_map;//only for fast check for existence
    tr1::unordered_map<int, int> selected_query_map;//map value is query node id, value is its order
    //add it to the selected list
    Set_Iterator *isi = new Set_Iterator();

    //if the restricted node ID exists, then limit its domain to the restricted node
    if (restricted_domain_id == -1) {
        // 获取大图上，与子图中nodesOrder[0]结点对应的label一致的结点
        isi->se = graph->get_nodes_by_label(this->get_node_with_id(nodes_order[0])->get_label());
        if (isi->se == NULL) {
            delete isi;
            return;
        }
    } else {
        isi->se = new set<int>();
        // insert the restricted_node_id first
        isi->se->insert(restricted_node_id);
    }
    isi->it = isi->se->begin();
    selected.push_back(isi);
    // save all the selected candidates into the set: selectedDataMap中
    selected_data_map.insert(*(isi->it));
    // 结点编号->选中的次序
    selected_query_map.insert(std::pair<int, int>(nodes_order[selected.size() - 1], selected.size() - 1));

    while (true) {
        num_iterations++;
        // postponedNodes为域到域中结点集合的映射
        if (postponed_nodes != 0) {
            if (num_iterations > max_iters) {
                tr1::unordered_map<int, tr1::unordered_set<int> *>::iterator iter = postponed_nodes->find(
                        restricted_domain_id);
                tr1::unordered_set<int> *nodes_list;

                // postponedNodes中不存在序号为restrictedDomainID的域，创建新的集合nodesList并插入
                if (iter == postponed_nodes->end()) {
                    nodes_list = new tr1::unordered_set<int>();
                    postponed_nodes->insert(
                            std::pair<int, tr1::unordered_set<int> *>(restricted_domain_id, nodes_list));
                }
                    // 否则令集合nodesList为postponedNodes中restrictedDomainID域对应的结点集合
                else {
                    nodes_list = iter->second;
                }
                nodes_list->insert(restricted_node_id);

                if (Settings::debug_msg) {
                    cout << "is_isomorphic function exceeded the allowed processing limit! NodeID: "
                         << restricted_node_id << ", DomainID: " << restricted_domain_id << endl;
                    cout << "max_iters = " << max_iters << ", num_iterations = " << num_iterations << endl;
                }

                {
                    vector<Set_Iterator *>::iterator iter = selected.begin();
                    if (iter != selected.end()) {
                        if (restricted_domain_id != -1)
                            delete (*iter)->se;
                        delete (*iter);
                        iter++;
                        for (; iter != selected.end(); iter++) {
                            (*iter)->se->clear();
                            delete (*iter)->se;
                            delete (*iter);
                        }
                    }
                }

                return;
            }
        }

        //take care of the counting part
        Set_Iterator *current_isi = selected.back();//.at(selected.size()-1);

        while (current_isi->is_iter_end()) {
            num_iterations++;

            //if we finished the domain of the first node
            if (selected.size() == 1) {
                if (restricted_domain_id != -1) {
                    delete (*(selected.begin()))->se;
                }
                delete (*(selected.begin()));
                selected.clear();
                return;
            }

            //clear the data node associated with the last selected
            //selected_data_map.erase(*(selected.back()->it));//commented on 27/10/2015
            //remove the last selected
            Set_Iterator *temp_to_del = selected.back();
            selected.pop_back();
            delete temp_to_del->se;
            delete temp_to_del;
            //delete the query node associated with the last selected
            selected_query_map.erase(nodes_order[selected.size()]);

            current_isi = selected.back();//.at(selected.size()-1);

            selected_data_map.erase(*(current_isi->it));
            current_isi->it++;
            if (!current_isi->is_iter_end())//only the if line is added on 27/10/2015
                selected_data_map.insert(*(current_isi->it));
        }
        //check current status
        NodeX *query_graph_node = get_node_with_id(nodes_order[selected.size() - 1]);
        num_iterations += query_graph_node->get_edges_size();

        bool b = check_current_status(this, graph, selected, nodes_order, selected_query_map);
        //if valid
        if (b) {
            //I found a solution
            if (selected.size() == this->get_num_of_nodes()) {
                map<int, int> *m = new map<int, int>();
                int c = 0;
                for (vector<Set_Iterator *>::iterator i = selected.begin(); i != selected.end(); ++i, c++) {
                    m->insert(std::pair<int, int>(nodes_order[c], *((*i)->it)));
                }
                results.push_back(m);

                if (restricted_domain_id != -1) {
                    //delete elements in selected
                    for (vector<Set_Iterator *>::iterator iter = selected.begin(); iter != selected.end(); iter++) {
                        (*iter)->se->clear();
                        delete (*iter)->se;
                        delete (*iter);
                    }
                    return;
                }

                selected_data_map.erase(*(selected.back()->it));
                selected.back()->it++;
                if (!selected.back()->is_iter_end())
                    selected_data_map.insert(*(selected.back()->it));
            } else//no solution found yet!
            {
                Set_Iterator *si = new Set_Iterator();
                si->se = new set<int>();
                //get extension from current selected (last si)
                //get the index of a query node connected to the query node to be selected for the check
                NodeX *temp = NULL;
                // tempN1为当前子图上的节点
                NodeX *temp_n1 = this->get_node_with_id(nodes_order[selected.size()]);
                vector<Set_Iterator *>::iterator temp_it = selected.begin();
                for (int i = 0; i < selected.size(); i++, temp_it++) {
                    // 找到子图上与tempN1相连的节点，找到该点对应在大图上的对应域的结点temp
                    if (temp_n1->is_it_connected_with_node_id(nodes_order[i])) {
                        temp = graph->get_node_with_id((*(*temp_it)->it));
                        break;
                    }
                }
                for (tr1::unordered_map<int, void *>::iterator iter = temp->get_edges_iterator(); iter !=
                                                                                                  temp->get_edges_end_iterator(); ++iter) {
                    num_iterations++;

                    if (num_iterations > max_iters)
                        break;

                    // 遍历temp的邻接结点otherNode
                    int other_node_id = iter->first;

                    NodeX *other_node = graph->get_node_with_id(other_node_id);

                    //check for the AC_3ed domain, to check for node occurrence, if not then no need to check it
                    if (prune_by_domain_values && domains_values.size() > 0) {
                        tr1::unordered_set<int> *temp_d = domains_values[nodes_order[selected.size()]];
                        // 只有otherNode在tempN1的域(即domains_values[nodes_order[selected.size()]])中才继续
                        if (temp_d->find(other_node_id) == temp_d->end()) {
                            continue;
                        }
                    }

                    // 只有otherNode与tempN1的label相等才继续
                    //check for node label
                    if (other_node->get_label() != temp_n1->get_label()) {
                        continue;
                    }

                    if (selected_data_map.find(other_node_id) == selected_data_map.end()) {
                        // 如果otherNode仍未被选中
                        bool b = true;
                        //check for other edges
                        // tempNi与其他结点相连，检查对应的大图上的结点otherNode与相应的结点是否也相连
                        for (tr1::unordered_map<int, void *>::iterator iter1 = temp_n1->get_edges_iterator(); iter1 !=
                                                                                                              temp_n1->get_edges_end_iterator(); ++iter1) {
                            num_iterations++;

                            if (num_iterations > max_iters)
                                break;

                            EdgeX *edge = (EdgeX *) iter1->second;
                            tr1::unordered_map<int, int>::iterator itr = selected_query_map.find(
                                    edge->get_other_node()->get_id());
                            if (itr == selected_query_map.end()) {
                                continue;
                            }
                            int temp_order = itr->second;
                            double edge_label = edge->get_label();
                            // 检查otherNode是否与相应的结点以edgeLabel标签的边相连
                            if (!other_node->is_it_connected_with_node_id((*(selected[temp_order]->it)), edge_label)) {
                                b = false;
                                break;
                            }
                        }

                        if (b) {
                            si->se->insert(other_node_id);
                        }
                    } else {
                    }
                }

                //I discovered a bug when testing the efficient graph, this fix should work now, otherwise the old code is below
                if (si->se->size() > 0) {
                    si->it = si->se->begin();
                    selected.push_back(si);
                    selected_data_map.insert(*(si->it));
                    selected_query_map.insert(
                            std::pair<int, int>(nodes_order[selected.size() - 1], selected.size() - 1));
                } else {
                    si->it = si->se->end();
                    delete si->se;
                    delete si;
                    //
                    selected_data_map.erase(*(selected.back()->it));
                    selected.back()->it++;
                    if (!selected.back()->is_iter_end())
                        selected_data_map.insert(*(selected.back()->it));
                }
            }
        } else {
            selected_data_map.erase(*(selected.back()->it));
            selected.back()->it++;
            if (!selected.back()->is_iter_end())
                selected_data_map.insert(*(selected.back()->it));
        }
    }

    //delete elements in selected
    vector<Set_Iterator *>::iterator iter = selected.begin();
    if (iter != selected.end()) {
        if (restricted_domain_id != -1)
            delete (*iter)->se;
        delete (*iter);
        iter++;
        for (; iter != selected.end(); iter++) {
            (*iter)->se->clear();
            delete (*iter)->se;
            delete (*iter);
        }
    }
    selected.clear();
}

/**
 * get the canonical label of this graph
 */
string GraphX::get_canonical_label() {
    if (CL.length() > 0) {
        return CL;
    }

    CL = CanonicalLabel::generate(this);

    return CL;
}

ostream &operator<<(ostream &os, const GraphX &g) {
    os << "# t 1\n";
    //output the nodes
    for (tr1::unordered_map<int, NodeX *>::const_iterator ii = g.nodes.begin(); ii != g.nodes.end(); ++ii) {
        NodeX *node = ii->second;
        os << "v " << node->get_id() << " " << node->get_label() << "\n";
    }

    //output the edges
    tr1::unordered_set<string> saved_edges;//list to keep track of already saved edges
    for (tr1::unordered_map<int, NodeX *>::const_iterator ii = g.nodes.begin(); ii != g.nodes.end(); ++ii) {
        NodeX *node = ii->second;
        for (tr1::unordered_map<int, void *>::iterator iter1 = node->get_edges_iterator(); iter1 !=
                                                                                           node->get_edges_end_iterator(); iter1++) {
            EdgeX *edge = (EdgeX *) iter1->second;

            //check whether it has been added before or not
            string sig =
                    int_to_string(node->get_id()) + "_" + double_to_string(edge->get_label()) + "_" + int_to_string(
                            edge->get_other_node()->get_id());
            if (node->get_id() < edge->get_other_node()->get_id())
                sig = int_to_string(edge->get_other_node()->get_id()) + "_" + double_to_string(edge->get_label()) +
                      "_" +
                      int_to_string(
                              node->get_id());
            if (saved_edges.find(sig) == saved_edges.end()) {
                saved_edges.insert(sig);
                os << "e " << node->get_id() << " " << edge->get_other_node()->get_id() << " " << setprecision(10)
                   << edge->get_label() << "\n";
            }
        }
    }

    return os;
}

//Destructor
GraphX::~GraphX() {
    for (tr1::unordered_map<int, NodeX *>::iterator ii = nodes.begin(); ii != nodes.end(); ++ii) {
        delete ii->second;
    }
    nodes.clear();

    for (tr1::unordered_map<double, set<int> *>::iterator ii = nodes_by_label.begin();
         ii != nodes_by_label.end(); ++ii) {
        delete ii->second;
    }
    nodes_by_label.clear();
}
