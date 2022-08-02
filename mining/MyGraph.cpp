/**
 * A representation of a graph, consisting of a set of nodes, and each node has a set of edges
 */

#include "MyEdge.h"
#include "MyGraph.h"
#include "Pattern.h"
#include "Settings.h"
#include "core_file.h"
#include "mining_utils.h"
#include "Signature.h"

bool check_current_status(MyGraph *graph1, MyGraph *graph2, std::vector<SetIter *> &current_s, int *nodes_order,
                          std::tr1::unordered_map<int, int> &selected_query_map) {
    MyNode *data_graph_node = graph2->get_node_with_id(*(current_s[current_s.size() - 1]->it));
    MyNode *query_graph_node = graph1->get_node_with_id(nodes_order[current_s.size() - 1]);

    for (std::tr1::unordered_map<int, void *>::iterator iter = query_graph_node->get_edges_begin(); iter !=
            query_graph_node->get_edges_end(); ++iter) {
        int other_node_id = iter->first;
        MyEdge *edge = (MyEdge *) (iter->second);

        if (selected_query_map.find(other_node_id) == selected_query_map.end())
            continue;

        // get the id of the other node in data graph
        int data_node_id = *(current_s[selected_query_map.find(other_node_id)->second]->it);

        //check whether there is an edge between data_graph_node and data_node_id
        if (!data_graph_node->is_connected_with(data_node_id, edge->get_label())) {
            //cout<<"false"<<endl;
            return false;
        }
    }

    return true;
}

void insert_into_edge_freq(MyNode *src, int dest_node_id, std::string dest_node_label, std::string edge_label,
                           std::tr1::unordered_map<std::string, void *> &edge_to_freq,
                           std::map<std::string, std::map<int, std::set<int>>> &freq_edge_pairs, bool add_src_only) {
    std::string key;
    if (src->get_label() > dest_node_label) {
        std::stringstream ss;
        if (src->get_label() == dest_node_label)
            ss << src->get_label() << dest_node_label << "," << edge_label << ",";
        else
            ss << src->get_label() << "," << dest_node_label << "," << edge_label << ",";
        key = ss.str();
    } else {
        std::stringstream ss;
        if (dest_node_label == src->get_label())
            ss << dest_node_label << src->get_label() << "," << edge_label << ",";
        else
            ss << dest_node_label << "," << src->get_label() << "," << edge_label << ",";
        key = ss.str();
    }

    std::tr1::unordered_map<std::string, void *>::iterator iter = edge_to_freq.find(key);
    Pattern *pattern;
    if (iter == edge_to_freq.end()) {
        MyGraph *graph = new MyGraph();
        MyNode *node1 = graph->add_node(0, src->get_label());
        MyNode *node2 = graph->add_node(1, dest_node_label);
        graph->add_edge(node1, node2, edge_label);
        pattern = new Pattern(graph);
        delete graph;

        edge_to_freq[key] = pattern;

        std::set<int> st1 = std::set<int>();
        std::set<int> st2 = std::set<int>();
        st1.insert(src->get_id());
        st2.insert(dest_node_id);
        std::map<int, std::set<int>> mp = std::map<int, std::set<int>>();
        mp[dest_node_id] = st1;
        mp[src->get_id()] = st2;
        freq_edge_pairs[key] = mp;
    } else {
        if (freq_edge_pairs[key].find(src->get_id()) == freq_edge_pairs[key].end()) {
            freq_edge_pairs[key][src->get_id()] = std::set<int>();
            freq_edge_pairs[key][src->get_id()].insert(dest_node_id);
        } else {
            freq_edge_pairs[key][src->get_id()].insert(dest_node_id);
        }
        if (freq_edge_pairs[key].find(dest_node_id) == freq_edge_pairs[key].end()) {
            freq_edge_pairs[key][dest_node_id] = std::set<int>();
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

void insert_into_edge_freq(MyNode *src, int dest_node_id, std::string dest_node_label, std::string edge_label,
                           std::tr1::unordered_map<std::string, void *> &edge_to_freq, bool add_src_only) {
    std::string key;
    if (src->get_label() > dest_node_label) {
        std::stringstream ss;
        if (src->get_label() == dest_node_label)
            ss << src->get_label() << dest_node_label << "," << edge_label << ",";
        else
            ss << src->get_label() << "," << dest_node_label << "," << edge_label << ",";
        key = ss.str();
    } else {
        std::stringstream ss;
        if (dest_node_label == src->get_label())
            ss << dest_node_label << src->get_label() << "," << edge_label << ",";
        else
            ss << dest_node_label << "," << src->get_label() << "," << edge_label << ",";
        key = ss.str();
    }

    std::tr1::unordered_map<std::string, void *>::iterator iter = edge_to_freq.find(key);
    Pattern *pattern;
    if (iter == edge_to_freq.end()) {
        MyGraph *graph = new MyGraph();
        MyNode *node1 = graph->add_node(0, src->get_label());
        MyNode *node2 = graph->add_node(1, dest_node_label);
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

MyGraph::MyGraph() {
    init(0);
}

MyGraph::MyGraph(int id) {
    init(id);
}

void MyGraph::init(int id) {
    this->id = id;
    edges_num = 0;
    this->freq = -1;
}

MyGraph::MyGraph(MyGraph *graph) {
    this->id = graph->get_id();
    edges_num = graph->get_edges_num();

    //copy nodes
    for (std::tr1::unordered_map<int, MyNode *>::const_iterator iter = graph->get_nodes_iterator(); iter !=
                                                                                                    graph->get_nodes_end_iterator(); ++iter) {
        MyNode *oldNode = iter->second;
        this->add_node(oldNode->get_id(), oldNode->get_label());
    }

    //copy edges
    for (std::tr1::unordered_map<int, MyNode *>::const_iterator iter = graph->get_nodes_iterator(); iter !=
                                                                                                    graph->get_nodes_end_iterator(); ++iter) {
        MyNode *old_node = iter->second;
        MyNode *node = nodes.find(old_node->get_id())->second;

        for (std::tr1::unordered_map<int, void *>::iterator iter2 = old_node->get_edges_begin(); iter2 !=
                old_node->get_edges_end(); ++iter2) {
            MyEdge *edge = (MyEdge *) (iter2->second);
            node->add_edge(nodes.find(edge->get_neighbor()->get_id())->second, edge->get_label());
        }
    }
}

MyNode *MyGraph::add_node(int id, std::string label) {
    sig.clear();

    std::tr1::unordered_map<int, MyNode *>::iterator temp = nodes.find(id);
    if (temp != nodes.end())
        return temp->second;

    MyNode *node = new MyNode(id, label);

    nodes.insert(std::pair<int, MyNode *>(id, node));

    std::tr1::unordered_map<std::string, std::set<int> *>::iterator iter = nodes_by_label.find(label);
    if (iter == nodes_by_label.end()) {
        nodes_by_label.insert(std::pair<std::string, std::set<int> *>(label, new std::set<int>()));
        iter = nodes_by_label.find(label);
    }
    iter->second->insert(id);

    return node;
}

/**
 * the nodes must exist before adding the edge
 */
void MyGraph::add_edge(MyNode *src_node, MyNode *dest_node, std::string edge_label) {
    sig.clear();

    src_node->add_edge(dest_node, edge_label);

    dest_node->add_edge(src_node, edge_label);

    edges_num++;
}

void MyGraph::add_edge(int src_id, int dest_id, std::string edge_label) {
    sig.clear();

    MyNode *src_node;
    MyNode *dest_node;

    std::tr1::unordered_map<int, MyNode *>::iterator iter = nodes.find(src_id);
    if (iter != nodes.end())
        src_node = nodes[src_id];
    else
        return;

    iter = nodes.find(dest_id);
    if (iter != nodes.end())
        dest_node = nodes[dest_id];
    else
        return;

    src_node->add_edge(dest_node, edge_label);

    dest_node->add_edge(src_node, edge_label);

    edges_num++;

    return;
}

void MyGraph::remove_edge(int id1, int id2) {
    sig.clear();

    nodes[id1]->remove_edge(nodes[id2]);
    if (nodes[id1]->get_edges_size() == 0)
        remove_node_ignore_edges(id1);

    nodes[id2]->remove_edge(nodes[id1]);
    if (nodes[id2]->get_edges_size() == 0)
        remove_node_ignore_edges(id2);

    edges_num--;
}

/**
 * remove a node from the graph, ignoring the edge connecting to it
 */
void MyGraph::remove_node_ignore_edges(int node_id) {
    sig.clear();

    nodes_by_label.find(get_node_with_id(node_id)->get_label())->second->erase(node_id);
    delete nodes.find(node_id)->second;
    nodes.erase(node_id);
}

void MyGraph::add_edge(int src_id, int dest_id, std::string edge_label, std::tr1::unordered_map<std::string, void *> &edge_to_freq,
                       std::map<std::string, std::map<int, std::set<int>>> &freq_edge_pairs) {
    if (sig.length() > 0)
        sig.clear();

    MyNode *src_node;
    MyNode *dest_node;

    std::tr1::unordered_map<int, MyNode *>::iterator iter = nodes.find(src_id);
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

    insert_into_edge_freq(dest_node, src_id, src_node->get_label(), edge_label, edge_to_freq, freq_edge_pairs, false);
}

void MyGraph::add_edge(int src_id, int dest_id, std::string edge_label, std::tr1::unordered_map<std::string, void *> &edge_to_freq) {
    if (sig.length() > 0)
        sig.clear();

    MyNode *src_node;
    MyNode *dest_node;

    std::tr1::unordered_map<int, MyNode *>::iterator iter = nodes.find(src_id);
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

    insert_into_edge_freq(dest_node, src_id, src_node->get_label(), edge_label, edge_to_freq, false);
}

MyNode *MyGraph::get_node_with_id(int node_id) {
    std::tr1::unordered_map<int, MyNode *>::iterator iter = nodes.find(node_id);
    if (iter == nodes.end())
        return nullptr;
    else
        return iter->second;
}

std::set<int> *MyGraph::get_nodes_by_label(std::string label) {
    std::tr1::unordered_map<std::string, std::set<int> *>::iterator iter = nodes_by_label.find(label);
    if (iter == nodes_by_label.end())
        return nullptr;
    return iter->second;
}

std::tr1::unordered_map<int, MyNode *>::const_iterator MyGraph::get_nodes_iterator() {
    return nodes.begin();
}

std::tr1::unordered_map<int, MyNode *>::const_iterator MyGraph::get_nodes_end_iterator() {
    return nodes.end();
}

/**
 * get edge label from the scr node to the destination node
 * if either the src node is not found, or the detination node is not found from the src node, then return 0.0001
 */
std::string MyGraph::get_edge_label(int src_node_id, int dest_node_id) {
    MyNode *srcNode;
    std::tr1::unordered_map<int, MyNode *>::iterator temp = nodes.find(src_node_id);
    if (temp == nodes.end())
        return "";
    srcNode = (*temp).second;

    auto *edge = (MyEdge *) srcNode->get_edge_for_dest_node(dest_node_id);
    if (edge == nullptr)
        return "";

    return edge->get_label();
}

int MyGraph::get_nodes_num() {
    return this->nodes.size();
}

bool MyGraph::is_connected() {
    if (nodes.size() < 2)
        return true;

    //start from any node
    MyNode *node = nodes.begin()->second;
    std::map<int, MyNode *> visited;
    std::map<int, MyNode *> to_visit;

    to_visit.insert(std::pair<int, MyNode *>(node->get_id(), node));
    while (!to_visit.empty()) {
        //1- pop a node for the to be visited list, 2- remove it from to_visit, and 3- add it to the visited list
        node = to_visit.begin()->second;//1
        to_visit.erase(to_visit.begin());//2
        visited.insert(std::pair<int, MyNode *>(node->get_id(), node));//3
        //add its neighbors
        for (std::tr1::unordered_map<int, void *>::iterator iter = node->get_edges_begin(); iter !=
                node->get_edges_end(); ++iter) {
            int neighbor_id = iter->first;
            if (visited.find(neighbor_id) != visited.end())
                continue;
            MyEdge *edge = (MyEdge *) iter->second;
            to_visit.insert(std::pair<int, MyNode *>(neighbor_id, edge->get_neighbor()));
        }
    }

    if (visited.size() == nodes.size())
        return true;
    else
        return false;

}

bool MyGraph::same_with(MyGraph *other_g) {
    if (this->get_nodes_num() != other_g->get_nodes_num() ||
        this->get_edges_num() != other_g->get_edges_num()) {
        return false;
    }

    //compare using sig
    std::string this_sig = this->get_sig();
    std::string other_sig = other_g->get_sig();
    if (this_sig.compare(other_sig) != 0)
        return false;
    else {
        bool a = false;
        if (this_sig.at(0) == 'X') a = true;

        if (!a)
            return true;
    }

    std::vector<std::map<int, int> *> result;
    std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> domains_values;

    this->is_isomorphic(other_g, result, domains_values, -1, -1, nullptr, false);

    bool b;

    if (result.size() > 0) {
        b = true;
    } else {
        b = false;
    }

    for (auto & iter1 : result) {
        delete iter1;
    }
    result.clear();

    return b;
}

/**
 * check whether two graphs are isomorphic or not
 * if isomorphic, the mapping between query graph and data graph is built in 'result'
 */
void MyGraph::is_isomorphic(MyGraph *graph, std::vector<std::map<int, int> *> &results,
                            std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> &domains_values,
                            int restricted_domain_id, int restricted_node_id,
                            std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> *postponed_nodes,
                            bool prune_by_domain_values, unsigned long max_iters) {

    num_iterations = 0;

    //record the node which has been checked
    std::tr1::unordered_set<int> checked;
    int nodes_order[get_nodes_num()];//the result of this part

    //nodes order
    std::vector<int> to_check;

    //let the restricted node be the first to check
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
        MyNode *curr_node = nodes[current];

        int start = to_check.size();
        // put the neighbors of currNode into the 'toCheck' list
        for (std::tr1::unordered_map<int, void *>::iterator iter = curr_node->get_edges_begin(); iter !=
                curr_node->get_edges_end(); ++iter) {
            int other_id = iter->first;
            if (checked.find(other_id) == checked.end()) {
                //put them in order based on the degree
                std::vector<int>::iterator iter1 = to_check.begin();
                for (int i = 0; i < start; i++)
                    iter1++;
                for (; iter1 != to_check.end(); ++iter1) {
                    // put them based on the degree-ascending order
                    if (nodes[other_id]->get_edges_size() < nodes[(*iter1)]->get_edges_size())
                        break;
                }
                to_check.insert(iter1, other_id);
            }
        }
    }

    std::vector<SetIter *> selected;
    std::tr1::unordered_set<int> selected_data_map;//whether the node is selected
    std::tr1::unordered_map<int, int> selected_query_map;//query node id -> order
    SetIter *isi = new SetIter();

    //limit the domain to the restricted node
    if (restricted_domain_id == -1) {
        // get the nodes in data graph whose node is consistent with the nodesOrder[0] in query graph
        isi->se = graph->get_nodes_by_label(this->get_node_with_id(nodes_order[0])->get_label());
        if (isi->se == nullptr) {
            delete isi;
            return;
        }
    } else {
        isi->se = new std::set<int>();
        isi->se->insert(restricted_node_id);
    }
    isi->it = isi->se->begin();
    selected.push_back(isi);
    // save all the selected candidates into selected_data_map
    selected_data_map.insert(*(isi->it));
    // node id -> order
    selected_query_map.insert(std::pair<int, int>(nodes_order[selected.size() - 1], selected.size() - 1));

    while (true) {
        num_iterations++;
        // postponedNodes: domain ID -> id of domain nodes
        if (postponed_nodes != nullptr) {
            if (num_iterations > max_iters) {
                std::tr1::unordered_map<int, std::tr1::unordered_set<int> *>::iterator iter = postponed_nodes->find(
                        restricted_domain_id);
                std::tr1::unordered_set<int> *nodes_list;

                // there are no domain whose id is restrictedDomainID, create new set nodesList and insert
                if (iter == postponed_nodes->end()) {
                    nodes_list = new std::tr1::unordered_set<int>();
                    postponed_nodes->insert(
                            std::pair<int, std::tr1::unordered_set<int> *>(restricted_domain_id, nodes_list));
                }
                // or let the set nodesList be the set corresponding to the restrictedDomainID of postponedNodes
                else {
                    nodes_list = iter->second;
                }
                nodes_list->insert(restricted_node_id);

                if (Settings::debug_msg) {
                    std::cout << "is_isomorphic function exceeded the allowed processing limit! NodeID: "
                         << restricted_node_id << ", DomainID: " << restricted_domain_id << std::endl;
                    std::cout << "max_iters = " << max_iters << ", num_iterations = " << num_iterations << std::endl;
                }

                {
                    auto selected_iter = selected.begin();
                    if (selected_iter != selected.end()) {
                        if (restricted_domain_id != -1)
                            delete (*selected_iter)->se;
                        delete (*selected_iter);
                        selected_iter++;
                        for (; selected_iter != selected.end(); selected_iter++) {
                            (*selected_iter)->se->clear();
                            delete (*selected_iter)->se;
                            delete (*selected_iter);
                        }
                    }
                }

                return;
            }
        }

        SetIter *current_isi = selected.back();

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
            //selected_data_map.erase(*(selected.back()->it));
            //remove the last selected
            SetIter *temp_to_del = selected.back();
            selected.pop_back();
            delete temp_to_del->se;
            delete temp_to_del;
            //delete the query node associated with the last selected
            selected_query_map.erase(nodes_order[selected.size()]);

            current_isi = selected.back();//.at(selected.size()-1);

            selected_data_map.erase(*(current_isi->it));
            current_isi->it++;
            if (!current_isi->is_iter_end())
                selected_data_map.insert(*(current_isi->it));
        }
        //check current status
        MyNode *query_graph_node = get_node_with_id(nodes_order[selected.size() - 1]);
        num_iterations += query_graph_node->get_edges_size();

        bool b = check_current_status(this, graph, selected, nodes_order, selected_query_map);
        //if valid
        if (b) {
            //a solution is found
            if (selected.size() == this->get_nodes_num()) {
                auto *m = new std::map<int, int>();
                int c = 0;
                for (auto i = selected.begin(); i != selected.end(); ++i, c++) {
                    m->insert(std::pair<int, int>(nodes_order[c], *((*i)->it)));
                }
                results.push_back(m);

                if (restricted_domain_id != -1) {
                    //delete elements in selected
                    for (auto & iter : selected) {
                        iter->se->clear();
                        delete iter->se;
                        delete iter;
                    }
                    return;
                }

                selected_data_map.erase(*(selected.back()->it));
                selected.back()->it++;
                if (!selected.back()->is_iter_end())
                    selected_data_map.insert(*(selected.back()->it));
            } else//no solution found yet
            {
                auto *si = new SetIter();
                si->se = new std::set<int>();
                //get extension from current selected (last si)
                //get the index of a query node connected to the query node to be selected for the check
                MyNode *temp = nullptr;
                // tempN1 is the node in query graph
                MyNode *temp_n1 = this->get_node_with_id(nodes_order[selected.size()]);
                auto temp_it = selected.begin();
                for (int i = 0; i < selected.size(); i++, temp_it++) {
                    // Find the selected node connected to tempN1 on the subgraph,
                    // and find the node 'temp' correspondingly on the data graph
                    if (temp_n1->is_connected_with(nodes_order[i])) {
                        temp = graph->get_node_with_id((*(*temp_it)->it));
                        break;
                    }
                }
                for (std::tr1::unordered_map<int, void *>::iterator iter = temp->get_edges_begin(); iter !=
                        temp->get_edges_end(); ++iter) {
                    num_iterations++;

                    if (num_iterations > max_iters)
                        break;

                    // traverse temp's adjacent nodes otherNode
                    int other_node_id = iter->first;

                    MyNode *other_node = graph->get_node_with_id(other_node_id);

                    if (prune_by_domain_values && !domains_values.empty()) {
                        std::tr1::unordered_set<int> *temp_d = domains_values[nodes_order[selected.size()]];
                        // continue only if 'otherNode' is in the domain of tempN1 (ie domains_values[nodes_order[selected.size()]])
                        if (temp_d->find(other_node_id) == temp_d->end()) {
                            continue;
                        }
                    }

                    // continue only if otherNode is equal to the label of tempN1
                    if (other_node->get_label() != temp_n1->get_label()) {
                        continue;
                    }

                    if (selected_data_map.find(other_node_id) == selected_data_map.end()) {
                        // If otherNode is still not selected
                        bool b = true;
                        // tempN1 is connected to other nodes, check whether the node 'otherNode' on data graph
                        // is also connected to the corresponding node
                        for (std::tr1::unordered_map<int, void *>::iterator iter1 = temp_n1->get_edges_begin(); iter1 !=
                                temp_n1->get_edges_end(); ++iter1) {
                            num_iterations++;

                            if (num_iterations > max_iters)
                                break;

                            MyEdge *edge = (MyEdge *) iter1->second;
                            std::tr1::unordered_map<int, int>::iterator itr = selected_query_map.find(
                                    edge->get_neighbor()->get_id());
                            if (itr == selected_query_map.end()) {
                                continue;
                            }
                            int temp_order = itr->second;
                            std::string edge_label = edge->get_label();
                            // check if 'otherNode' is connected to the corresponding node with the edge labeled 'edgeLabel'
                            if (!other_node->is_connected_with((*(selected[temp_order]->it)), edge_label)) {
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
    auto iter = selected.begin();
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
std::string MyGraph::get_sig() {
    if (sig.length() > 0) {
        return sig;
    }

    sig = Signature::generate(this);

    return sig;
}

std::ostream &operator<<(std::ostream &os, const MyGraph &g) {
    os << "# t 1\n";
    //output the nodes
    for (auto ii : g.nodes) {
        MyNode *node = ii.second;
        os << "v " << node->get_id() << " " << node->get_label() << "\n";
    }

    //output the edges
    std::tr1::unordered_set<std::string> saved_edges;//list to keep track of already saved edges
    for (auto ii : g.nodes) {
        MyNode *node = ii.second;
        for (std::tr1::unordered_map<int, void *>::iterator iter1 = node->get_edges_begin(); iter1 !=
                node->get_edges_end(); iter1++) {
            auto *edge = (MyEdge *) iter1->second;

            //check whether it has been added before or not
            std::string sig =
                    int_to_string(node->get_id()) + "_" + edge->get_label() + "_" + int_to_string(
                            edge->get_neighbor()->get_id());
            if (node->get_id() < edge->get_neighbor()->get_id())
                sig = int_to_string(edge->get_neighbor()->get_id()) + "_" + edge->get_label() +
                      "_" +
                      int_to_string(
                              node->get_id());
            if (saved_edges.find(sig) == saved_edges.end()) {
                saved_edges.insert(sig);
                os << "e " << node->get_id() << " " << edge->get_neighbor()->get_id() << " " << std::setprecision(10)
                   << edge->get_label() << "\n";
            }
        }
    }

    return os;
}

//Destructor
MyGraph::~MyGraph() {
    for (std::tr1::unordered_map<int, MyNode *>::iterator ii = nodes.begin(); ii != nodes.end(); ++ii) {
        delete ii->second;
    }
    nodes.clear();

    for (std::tr1::unordered_map<std::string, std::set<int> *>::iterator ii = nodes_by_label.begin();
         ii != nodes_by_label.end(); ++ii) {
        delete ii->second;
    }
    nodes_by_label.clear();
}
