#include "Settings.h"
#include "core_file.h"
#include "CSPSolver.h"
#include "mining_utils.h"

std::string get_sig(int a, int b, std::string el) {
    char ch[20];

    std::string sig;
    if (a < b)
        sig = int_to_string(a) + "_" + int_to_string(b) + "_" + el;
    else
        sig = int_to_string(b) + "_" + int_to_string(a) + "_" + el;
    return sig;
}

void delete_results(std::vector<std::map<int, int> *> &result) {
    for (auto &iter1: result) {
        delete iter1;
    }
    result.clear();
}

/**
 * given subgraph 'pattern', the function decide whether the subgraph is frequent or not
 * @param graph the single large graph
 * @param pattern the subgraph tobe decided
 * @param support the given threshold
 * @param approximate decide whether we allow approximation or not
 * @param domains_solutions the mapping nodes of each node in subgraph
 * @return the frequency of the subgraph
 */
int CSPSolver::get_frequency(MyGraph *graph, Pattern *pattern, int support, double approximate,
                             std::map<int, std::set<int>> &domains_solutions) {
    int freq = 0;

    // domain_values: pNodeID -> set(gNodeID)
    std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> domains_values;
    MyGraph *pg = pattern->get_graph();
    for (std::tr1::unordered_map<int, MyNode *>::const_iterator iter = pg->get_nodes_iterator(); iter !=
                                                                                                 pg->get_nodes_end_iterator(); ++iter) {
        int varNodeID = iter->first;
        domains_values.insert(
                std::pair<int, std::tr1::unordered_set<int> *>(varNodeID, new std::tr1::unordered_set<int>()));
    }


    //insert domains values according to node consistency and node degree
    for (std::tr1::unordered_map<int, std::tr1::unordered_set<int> *>::iterator iter = domains_values.begin();
         iter != domains_values.end(); iter++) {
        //for each node in subgraph
        MyNode *p_node = pattern->get_graph()->get_node_with_id(iter->first);
        std::tr1::unordered_set<int> *current_domain = domains_values[p_node->get_id()];

        //check node consistency
        std::set<int> *nodes_same_label = graph->get_nodes_by_label(p_node->get_label());

        for (int iter1: *nodes_same_label) {
            MyNode *d_node = graph->get_node_with_id(iter1);
            if (d_node->get_edges_size() >= p_node->get_edges_size())    // [*] apply neighborhood consistency
            {
                current_domain->insert(d_node->get_id());
            }
        }
        if (current_domain->size() < support)
            return 0;
    }

    int pattern_size = pattern->get_graph()->get_nodes_num();
    long long start = get_msec();

    //do some filter based on arc consistency constraint
    check_ac(graph, domains_values, pattern, support);

    long long end = get_msec();
    long long elapsed = end - start;
    for (int i = 0; i < pattern_size; i++) {
        // if domain size < support， delete it
        if (domains_values.find(i)->second->size() < support) {
            for (auto &domains_value: domains_values)
                delete domains_value.second;
            return 0;
        }
    }

    int *ordering = new int[pattern_size];

    // init the order based on the pattern size
    for (int i = 0; i < pattern_size; i++) {
        ordering[i] = i;
    }

    //create solutions for each pattern node
    for (int i = 0; i < pattern_size; i++) {
        domains_solutions[i] = std::set<int>();
    }

    //use unique labels optimization
    if (pattern->has_unique_labels() && CSPSolver::check_acyclic(*(pattern->get_graph())))//CSPSolver::use_ac3
    {
        int min = std::numeric_limits<int>::max();
        bool flag = false;
        // find the min size of all domains
        for (auto &domains_value: domains_values) {
            flag = true;
            int temp = domains_value.second->size();
            if (temp < min)
                min = temp;
        }
        if (!flag) {
            min = 0;
        }
        if (min >= support) {
            for (auto &domains_value: domains_values) {
                for (int x: *domains_value.second) {
                    domains_solutions[domains_value.first].insert(x);
                }
            }
        }
        for (auto &domains_value: domains_values) {
            delete domains_value.second;
        }

        return min;
    }

    auto *postponed_nodes = new std::tr1::unordered_map<int, std::tr1::unordered_set<int> *>();
    if (!Settings::throw_nodes_after_iterations) {
        postponed_nodes = nullptr;
    }

    for (int i = 0; i < pattern_size; i++) {
        // domain_id is the subgraph's node which is being processed（The domain of this node is currently being processed）
        int domain_id = ordering[i];

        // [*] apply automorphism

        // current_domain is the domain corresponding to the subgraph node currently being processed
        std::tr1::unordered_set<int> *current_domain = domains_values.find(domain_id)->second;

        //if (Settings::debug_msg)
        //    cout << "old current_domain size:" << current_domain->size() << endl;

        // allow approximate mining
        if (approximate != -1) {
            std::map<int, int> id_map;
            int c = 0;
            for (int iter : *current_domain) {
                id_map.insert(std::pair<int, int>(c, iter));
                c++;
            }
            delete current_domain;
            domains_values.erase(domain_id);
            current_domain = new std::tr1::unordered_set<int>();
            domains_values.insert(std::pair<int, std::tr1::unordered_set<int> *>(domain_id, current_domain));

            // sample the amount of "size()*approximate" nodes to be computed later
            while (current_domain->size() < (id_map.size() * approximate)) {
                int r = rand() % id_map.size();
                current_domain->insert(id_map.at(r));
            }
        }

        //if (Settings::debug_msg)
        //    cout << "new current_domain size:" << current_domain->size() << endl;

        int cnt = 0;
        for (std::tr1::unordered_set<int>::iterator iter = current_domain->begin();
             iter != current_domain->end(); ++iter) {
            //cout<<"DID:"<<domain_id<<", c:"<<cnt<<endl;
            cnt++;
            int node_id = (*iter);
            bool b = false;
            std::vector<std::map<int, int> *> result;
            // check whether this node is already in solutions
            if (domains_solutions.find(domain_id)->second.find(node_id) !=
                domains_solutions.find(domain_id)->second.end()) {
                b = true;
            } else {
                pattern->get_graph()->is_isomorphic(graph, result, domains_values, domain_id, node_id, postponed_nodes);
                if (result.size() > 0)
                    b = true;
                else {
                    if (approximate == -1) {
                        // the remaining nodes would not satisfy the threshold
                        if (current_domain->size() - cnt + domains_solutions.find(domain_id)->second.size() <
                            support) {
                            delete_results(result);
                            break;
                        }
                    } else {
                        // compare in the approximate situation
                        if (current_domain->size() - cnt + domains_solutions.find(domain_id)->second.size() <
                            (support * approximate)) {
                            delete_results(result);
                            break;
                        }
                    }
                }
            }
            if (b)    //find valid node in current_domain
            {
                if (approximate == -1) {
                    for (auto current_mapping: result) {
                        // Traverse all the mapping nodes on the large graph that are isomorphic to the subgraph nodes,
                        // and insert them into domain_solution
                        // (mark all the values in the solution in the corresponding domain)
                        for (auto &iter2: *current_mapping) {
                            int dID = iter2.first;
                            int nID = iter2.second;
                            if (domains_solutions.find(dID) == domains_solutions.end()) {
                                domains_solutions[dID] = std::set<int>();
                            }
                            domains_solutions[dID].insert(nID);
                        }
                    }
                } else {
                    if (domains_solutions.find(domain_id) == domains_solutions.end()) {
                        domains_solutions[domain_id] = std::set<int>();
                    }
                    domains_solutions[domain_id].insert(node_id);
                }
            }
            if (approximate == -1) {
                if (domains_solutions[domain_id].size() >= support) {
                    //if (Settings::debug_msg)
                    //    for (auto domain_solution: domains_solutions)
                    //        cout << domain_solution.second.size() << endl;
                    delete_results(result);
                    break;
                }
            } else {
                if (domains_solutions[domain_id].size() / approximate >= support) {
                    delete_results(result);
                    break;
                }
            }

            delete_results(result);
        }

        if (approximate == -1) {
            if (domains_solutions[domain_id].size() < support)
                break;
        } else {
            int tf = domains_solutions[domain_id].size() / approximate;
            if (tf < support) {
                break;
            }
        }
    }

    //if (Settings::debug_msg) {
    //    cout << *pattern->get_graph();
    //    for (auto &domains_solution: domains_solutions) {
    //        cout << "domain ID: " << domains_solution.first << "domain size: " << domains_solution.second.size()
    //             << endl;
    //        for (auto &value_iter: domains_solution.second) {
    //            cout << "value ID: " << value_iter << endl;
    //        }
    //    }
    //}

    //delete the domains
    for (auto &domains_value: domains_values) {
        delete domains_value.second;
    }

    // iterate all the domains and get MNI frequency
    if (approximate == -1) {
        int min = std::numeric_limits<int>::max();
        bool flag = false;
        for (auto &domains_solution: domains_solutions) {
            flag = true;
            int temp = domains_solution.second.size();
            if (temp < min)
                min = temp;
        }
        if (!flag) {
            freq = 0;
        } else {
            freq = min;
        }
    } else {
        int min = std::numeric_limits<int>::max();
        for (auto &domains_solution: domains_solutions) {
            int domainID = domains_solution.first;
            int temp = domains_solution.second.size();
            //temp = domains_values.find(domainID)->second->size();
            temp = temp / approximate;
            if (temp < min)
                min = temp;
        }
        freq = min;
    }


    //delete domains solutions
//	for(auto & domains_solution : domains_solutions)
//	{
//		domains_solution.second.clear();
//	}

    //delete the ordering
    delete ordering;

    return freq;
}


void
CSPSolver::check_ac(MyGraph *graph, std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> &domains_values,
                    Pattern *pattern,
                    int support) {
    CSPSolver::check_ac(graph, domains_values, pattern->get_graph(), support, pattern->get_invalid_col());
}

void
CSPSolver::check_ac(MyGraph *graph, std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> &domains_values,
                    MyGraph *p_graph,
                    int support, int invalid_col) {
    std::map<std::string, EdgeInfo *> arcs;

    for (std::tr1::unordered_map<int, MyNode *>::const_iterator iter = p_graph->get_nodes_iterator(); iter !=
                                                                                                      p_graph->get_nodes_end_iterator(); ++iter) {
        MyNode *pNode = iter->second;
        for (std::tr1::unordered_map<int, void *>::iterator iter1 = pNode->get_edges_begin(); iter1 !=
                                                                                              pNode->get_edges_end(); iter1++) {
            MyEdge *edge = (MyEdge *) (iter1->second);
            int id1 = pNode->get_id();
            int id2 = edge->get_neighbor()->get_id();
            std::string sig = get_sig(id1, id2, edge->get_label());

            if (arcs.find(sig) != arcs.end())
                continue;

            EdgeInfo *edge_info = new EdgeInfo();
            edge_info->id1 = id1;
            edge_info->id2 = id2;
            edge_info->edge_label = edge->get_label();
            edge_info->min_domain_size = domains_values.find(edge_info->id1)->second->size();
            if (edge_info->min_domain_size < domains_values.find(edge_info->id2)->second->size())
                edge_info->min_domain_size = domains_values.find(edge_info->id2)->second->size();

            auto arc = arcs.begin();
            for (; arc != arcs.end(); arc++) {
                EdgeInfo *temp_ei = arc->second;
                if (temp_ei->min_domain_size > edge_info->min_domain_size)
                    break;
            }
            if (arcs.find(sig) != arcs.end())//new 2April
                delete edge_info;
            else
                arcs.insert(arc, std::pair<std::string, EdgeInfo *>(sig, edge_info));
        }
    }

    while (arcs.size() > 0) {
        EdgeInfo *edge_info = 0;
        std::string sig;

        if (invalid_col != -1) {
            auto arc = arcs.begin();
            for (; arc != arcs.end(); arc++) {
                EdgeInfo *temp_ei = arc->second;
                if (temp_ei->id1 == invalid_col || temp_ei->id2 == invalid_col) {
                    edge_info = arc->second;
                    sig = arc->first;
                    break;
                }
            }
        }

        if (edge_info == 0) {
            edge_info = arcs.begin()->second;
            sig = arcs.begin()->first;
        }

        std::tr1::unordered_set<int> *D1 = domains_values.find(edge_info->id1)->second;
        std::tr1::unordered_set<int> *D2 = domains_values.find(edge_info->id2)->second;
        int old_size1 = D1->size();
        int old_size2 = D2->size();

        if (refine(graph, D1, D2, edge_info, support)) {
            auto arc = arcs.begin();
            for (; arc != arcs.end(); arc++) {
                EdgeInfo *temp_ei = arc->second;
                delete temp_ei;
            }
            arcs.clear();
            return;
        }

        //add affected arcs
        MyNode *pNode = p_graph->get_node_with_id(edge_info->id1);
        if (old_size1 != D1->size() && pNode->get_edges_size() > 1) {
            int id1 = edge_info->id1;
            for (std::tr1::unordered_map<int, void *>::iterator iter1 = pNode->get_edges_begin(); iter1 !=
                                                                                                  pNode->get_edges_end(); iter1++) {
                MyEdge *edge = (MyEdge *) (iter1->second);

                int id2 = edge->get_neighbor()->get_id();
                if (id2 == edge_info->id2)
                    continue;
                std::string sig = get_sig(id1, id2, edge_info->edge_label);

                if (arcs.find(sig) != arcs.end());//delete edge_info;
                else {
                    //create edge_info
                    EdgeInfo *new_edge_info = new EdgeInfo();
                    new_edge_info->id1 = id1;
                    new_edge_info->id2 = id2;
                    new_edge_info->edge_label = edge->get_label();
                    new_edge_info->min_domain_size = domains_values.find(new_edge_info->id1)->second->size();
                    if (new_edge_info->min_domain_size < domains_values.find(new_edge_info->id2)->second->size())
                        new_edge_info->min_domain_size = domains_values.find(new_edge_info->id2)->second->size();

                    arcs.insert(std::pair<std::string, EdgeInfo *>(sig, new_edge_info));
                }
            }
        }

        pNode = p_graph->get_node_with_id(edge_info->id2);
        if (old_size2 != D2->size() && pNode->get_edges_size() > 1) {
            int id1 = edge_info->id2;
            for (std::tr1::unordered_map<int, void *>::iterator iter1 = pNode->get_edges_begin(); iter1 !=
                                                                                                  pNode->get_edges_end(); iter1++) {
                MyEdge *edge = (MyEdge *) (iter1->second);

                int id2 = edge->get_neighbor()->get_id();
                if (id2 == edge_info->id1)
                    continue;
                std::string sig = get_sig(id1, id2, edge_info->edge_label);

                if (arcs.find(sig) != arcs.end());//delete edge_info;
                else {
                    //create edge_info
                    EdgeInfo *new_edge_info = new EdgeInfo();
                    new_edge_info->id1 = id2;
                    new_edge_info->id2 = id1;
                    new_edge_info->edge_label = edge->get_label();
                    new_edge_info->min_domain_size = domains_values.find(new_edge_info->id1)->second->size();
                    if (new_edge_info->min_domain_size < domains_values.find(new_edge_info->id2)->second->size())
                        new_edge_info->min_domain_size = domains_values.find(new_edge_info->id2)->second->size();

                    arcs.insert(std::pair<std::string, EdgeInfo *>(sig, new_edge_info));
                }
            }
        }

        arcs.erase(sig);
        delete edge_info;
    }

    //clear arcs list
    auto iter = arcs.begin();
    for (; iter != arcs.end(); iter++) {
        EdgeInfo *temp_pwe = iter->second;
        delete temp_pwe;
    }
    arcs.clear();
}

/**
 * This method requires that for the smaller domain D1, all nodes in it must satisfy:
 * be able to find a node in D2 that is connected to this node by the edge with "edge_label",
 * otherwise it should be deleted the node in D1.
 *
 * At the same time, the node in D2 should also satisfy that a node can be found connected to it in D1
 * and the edge label is edge_label, otherwise it is also to be deleted
 *
 * After the above deletion,
 * if the domain size of either of D1 and D2 is less than the required support,
 * then return false, otherwise return true
 * @param graph
 * @param D1
 * @param D2
 * @param pwe
 * @param support
 * @return
 */
bool
CSPSolver::refine(MyGraph *graph, std::tr1::unordered_set<int> *D1, std::tr1::unordered_set<int> *D2, EdgeInfo *pwe,
                  int support) {
    std::string edgeLabel = pwe->edge_label;
    //set to iterate over the smaller domain
    if (D1->size() > D2->size()) {
        std::tr1::unordered_set<int> *temp = D1;
        D1 = D2;
        D2 = temp;
    }
    //go over each node in the domain1
    std::set<int> new_D2;
    for (std::tr1::unordered_set<int>::iterator iter = D1->begin(); iter != D1->end();) {
        MyNode *node = graph->get_node_with_id((*iter));
        //go over its edges
        bool deleteIt = true;
        for (std::tr1::unordered_map<int, void *>::iterator iter1 = node->get_edges_begin(); iter1 !=
                                                                                             node->get_edges_end(); ++iter1) {
            MyEdge *edge = (MyEdge *) (iter1->second);
            if (edge->get_label() != edgeLabel)
                continue;
            std::tr1::unordered_set<int>::iterator iter_f = D2->find(edge->get_neighbor()->get_id());
            if (iter_f != D2->end()) {
                // A node is found in D2 which is connected to the node in D1 by an edge labeled "edge_label"
                deleteIt = false;
                new_D2.insert(*iter_f);
            }
        }

        if (deleteIt) {
            iter = D1->erase(iter);
            if (D1->size() < support)
                return true;
        } else
            ++iter;
    }

    for (std::tr1::unordered_set<int>::iterator iter = D2->begin(); iter != D2->end();) {
        if (new_D2.find(*iter) == new_D2.end()) {
            iter = D2->erase(iter);
            if (D2->size() < support)
                return true;
        } else {
            iter++;
        }
    }

    return false;
}

/**
 * check whether the graph is acyclic or not
 * @param query
 * @return
 */
bool CSPSolver::check_acyclic(MyGraph &query) {
    std::tr1::unordered_set<int> visited;
    std::vector<int> to_be_visited;

    int current_node_id;
    MyNode *current_node;
    to_be_visited.push_back(0);
    while (visited.size() < query.get_nodes_num()) {
        if (to_be_visited.size() == 0)
            break;

        current_node_id = to_be_visited.at(0);
        current_node = query.get_node_with_id(current_node_id);

        to_be_visited.erase(to_be_visited.begin());
        visited.insert(current_node_id);
        int already_visited_neighbors = 0;

        for (std::tr1::unordered_map<int, void *>::iterator iter = current_node->get_edges_begin(); iter !=
                                                                                                    current_node->get_edges_end(); iter++) {
            MyEdge *edge = (MyEdge *) iter->second;
            int other_node_id = edge->get_neighbor()->get_id();

            if (visited.find(other_node_id) != visited.end()) {
                already_visited_neighbors++;
                if (already_visited_neighbors > 1) {
                    return false;
                }
            } else {
                to_be_visited.push_back(other_node_id);
            }
        }
    }

    return true;
}
