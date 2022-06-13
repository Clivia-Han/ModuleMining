#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <limits>
#include <math.h>
#include <cstdlib>
#include<sys/time.h>
#include "GraMiCounter.h"
#include "utils.h"
#include "Settings.h"

int GraMiCounter::num_by_passed_nodes;
int GraMiCounter::num_side_effect_nodes;
bool GraMiCounter::use_ac3;

//utility functions
long long get_ms_of_day_3() {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return (long long) tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

string get_sig(int a, int b, double el) {
    char ch[20];

    string sig;
    if (a < b)
        sig = int_to_string(a) + "_" + int_to_string(b) + "_" + double_to_string(el);
    else
        sig = int_to_string(b) + "_" + int_to_string(a) + "_" + double_to_string(el);
    return sig;
}

//function to delete results space
void delete_results(vector<map<int, int> *> &result) {
    for (auto &iter1: result) {
        delete iter1;
    }
    result.clear();
}

/**
 * ModuleMining based approximate counter, for the 'pattern' in the 'graph' given the 'support' minimum threshold.
 * the 'approximate' parameter indicates whether we allow approximation or not, if approximate==-1 then it is not approximate
 */
// 这里解决给定子图，计算支持度的问题，缺少子图扩展和候选集生成步骤，已用上了isFrequentHeuristic中的改进
// 实现了unique label优化策略（isFrequentAdv中有）
int GraMiCounter::is_frequent(GraphX *graph, Pattern *pattern, int support, double approximate,
                              map<int, set<int>> &domains_solutions) {
    int freq = 0;

    //create domains
    // domain_values: pNodeID -> set(gNodeID)
    tr1::unordered_map<int, tr1::unordered_set<int> *> domains_values;
    GraphX *pg = pattern->get_graph();
    for (tr1::unordered_map<int, NodeX *>::const_iterator iter = pg->get_nodes_iterator(); iter !=
                                                                                           pg->get_nodes_end_iterator(); ++iter) {
        int varNodeID = iter->first;
        domains_values.insert(std::pair<int, tr1::unordered_set<int> *>(varNodeID, new tr1::unordered_set<int>()));
    }


    //insert domains values (considering node, neighborhood count, and degree consistencies)
    for (tr1::unordered_map<int, tr1::unordered_set<int> *>::iterator iter = domains_values.begin();
         iter != domains_values.end(); iter++) {
        //get the pattern node
        NodeX *p_node = pattern->get_graph()->get_node_with_id(iter->first);
        tr1::unordered_set<int> *current_domain = domains_values[p_node->get_id()];

        //check node consistency
        set<int> *nodes_same_label = graph->get_nodes_by_label(p_node->get_label());

        for (int iter1: *nodes_same_label) {
            NodeX *d_node = graph->get_node_with_id(iter1);
            if (d_node->get_edges_size() >= p_node->get_edges_size())    // [*] apply neighborhood consistency
            {
                current_domain->insert(d_node->get_id());
            }
        }
        if (current_domain->size() < support)
            return 0;
    }

    int pattern_size = pattern->get_graph()->get_num_of_nodes();
    long long start = get_ms_of_day_3();

    //apply arc consistency constraint
    AC_3(graph, domains_values, pattern, support);

    long long end = get_ms_of_day_3();
    long long elapsed = end - start;
    //check for domains lengths
    for (int i = 0; i < pattern_size; i++) {
        if (domains_values.find(i)->second->size() < support) {
            for (auto &domains_value: domains_values)
                delete domains_value.second;

            return 0;
        }
    }

    // [*] set domains order
    int *ordering = new int[pattern_size];

    for (int i = 0; i < pattern_size; i++) {
        ordering[i] = i;
    }

    //create solutions data structure
    for (int i = 0; i < pattern_size; i++) {
        domains_solutions[i] = std::set<int>();
    }

    // 使用unique labels优化策略
    //check for unique labels optimization, if applicable then return true (as we already checked in the previous  step for infrequentness)
    if (pattern->has_unique_labels() && GraMiCounter::is_it_acyclic(*(pattern->get_graph())))//GraMiCounter::use_ac3
    {
        int min = std::numeric_limits<int>::max();
        bool flag = false;
        for (auto &domains_value: domains_values) {
            flag = true;
            int temp = domains_value.second->size();
            //delete iter->second;//new
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

    for (int i = 0; i < pattern_size; i++) {
        // domain_id is the subgraph's node which is being processed（The domain of this node is currently being processed）
        int domain_id = ordering[i];

        // [*] apply automorphism

        //go over elements in the current domain, check if a solution exists
        // current_domain is the domain corresponding to the subgraph node currently being processed
        tr1::unordered_set<int> *current_domain = domains_values.find(domain_id)->second;

        //if (Settings::debug_msg)
        //    cout << "old current_domain size:" << current_domain->size() << endl;

        if (approximate != -1) {
            map<int, int> id_map;
            int c = 0;
            for (tr1::unordered_set<int>::iterator iter = current_domain->begin();
                 iter != current_domain->end(); iter++) {
                id_map.insert(std::pair<int, int>(c, *iter));
                c++;
            }
            delete current_domain;
            domains_values.erase(domain_id);
            current_domain = new tr1::unordered_set<int>();
            domains_values.insert(std::pair<int, tr1::unordered_set<int> *>(domain_id, current_domain));

            // allows approximate mining
            // Randomly sample "size()*approximate" number of samples in the original domain,
            // and redefine current_domain to determine whether the domain meets the support requirements
            while (current_domain->size() < (id_map.size() * approximate)) {
                int r = rand() % id_map.size();
                current_domain->insert(id_map.at(r));
            }
        }

        //if (Settings::debug_msg)
        //    cout << "new current_domain size:" << current_domain->size() << endl;

        int counter = 0;
        for (tr1::unordered_set<int>::iterator iter = current_domain->begin(); iter != current_domain->end(); ++iter) {
            //cout<<"DID:"<<domain_id<<", c:"<<counter<<endl;
            counter++;
            // node_id is the large graph node in the domain
            // corresponding to the subgraph domain_id currently being processed
            int node_id = (*iter);
            bool b = false;
            vector<map<int, int> *> result;
            //check if this node has been passed previously as a correct solution or not
            if (domains_solutions.find(domain_id)->second.find(node_id) !=
                domains_solutions.find(domain_id)->second.end()) {
                b = true;
            } else {
                pattern->get_graph()->is_isomorphic(graph, result, domains_values, domain_id, node_id);
                if (result.size() > 0)
                    b = true;
                else {
                    //in case the remaining + existing solutions can not satisfy the support, break
                    if (approximate == -1) {
                        if (current_domain->size() - counter + domains_solutions.find(domain_id)->second.size() <
                            support) {
                            delete_results(result);
                            // In the case where approximate mining is not allowed,
                            // if the sum of (nodes to be detected + valid nodes) < support degree,
                            // the corresponding subgraph is infrequent
                            break;
                        }
                    } else {
                        if (current_domain->size() - counter + domains_solutions.find(domain_id)->second.size() <
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
                    //there is a solution for this node, add all valid node values to the solutions domain
                    for (auto current_mapping: result) {
                        // Traverse all the mapping nodes on the large graph that are isomorphic to the subgraph nodes,
                        // and insert them into domain_solution
                        // (mark all the values in the solution in the corresponding domain)
                        for (auto &iter2: *current_mapping) {
                            int dID = iter2.first;
                            int nID = iter2.second;
                            if (domains_solutions.find(dID) == domains_solutions.end()) {
                                domains_solutions[dID] = set<int>();
                            }
                            domains_solutions[dID].insert(nID);
                        }
                    }
                } else {
                    if (domains_solutions.find(domain_id) == domains_solutions.end()) {
                        domains_solutions[domain_id] = set<int>();
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

        //in case the solution can not satisfy the support, break
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

    //get MNI frequency using the domains solutions
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

    } else //in case of approximation
    {
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
GraMiCounter::AC_3(GraphX *graph, tr1::unordered_map<int, tr1::unordered_set<int> *> &domains_values, Pattern *pattern,
                   int support) {
    GraMiCounter::AC_3(graph, domains_values, pattern->get_graph(), support, pattern->get_invalid_col());
}

void
GraMiCounter::AC_3(GraphX *graph, tr1::unordered_map<int, tr1::unordered_set<int> *> &domains_values, GraphX *p_graph,
                   int support, int invalid_col) {
    map<string, pair_with_edge *> arcs;

    //create pairs of connection from the pattern
    for (tr1::unordered_map<int, NodeX *>::const_iterator iter = p_graph->get_nodes_iterator(); iter !=
                                                                                                p_graph->get_nodes_end_iterator(); ++iter) {
        NodeX *pNode = iter->second;
        for (tr1::unordered_map<int, void *>::iterator iter1 = pNode->get_edges_iterator(); iter1 !=
                                                                                            pNode->get_edges_end_iterator(); iter1++) {
            EdgeX *edge = (EdgeX *) (iter1->second);
            int id1 = pNode->get_id();
            int id2 = edge->get_other_node()->get_id();
            string sig = get_sig(id1, id2, edge->get_label());

            if (arcs.find(sig) != arcs.end())
                continue;

            pair_with_edge *pwe = new pair_with_edge();
            pwe->id1 = id1;
            pwe->id2 = id2;
            pwe->edge_label = edge->get_label();
            pwe->min_domain_size = domains_values.find(pwe->id1)->second->size();
            if (pwe->min_domain_size < domains_values.find(pwe->id2)->second->size())
                pwe->min_domain_size = domains_values.find(pwe->id2)->second->size();

            auto iter = arcs.begin();
            for (; iter != arcs.end(); iter++) {
                pair_with_edge *temp_pwe = iter->second;
                //cout<<iter->first<<endl<<flush;
                if (temp_pwe->min_domain_size > pwe->min_domain_size)
                    break;
            }
            if (arcs.find(sig) != arcs.end())//new 2April
                delete pwe;
            else
                arcs.insert(iter, std::pair<string, pair_with_edge *>(sig, pwe));
        }
    }

    //order arcs to start with invalidcolumn if exists

    while (arcs.size() > 0) {
        pair_with_edge *pwe = 0;
        string sig;

        if (invalid_col != -1) {
            auto iter = arcs.begin();
            for (; iter != arcs.end(); iter++) {
                pair_with_edge *temp_pwe = iter->second;
                if (temp_pwe->id1 == invalid_col || temp_pwe->id2 == invalid_col) {
                    pwe = iter->second;
                    sig = iter->first;
                    break;
                }
            }
        }

        if (pwe == 0) {
            pwe = arcs.begin()->second;
            sig = arcs.begin()->first;
        }

        //this lookup is repeated in the refine function, think of a way not to repeat it
        tr1::unordered_set<int> *D1 = domains_values.find(pwe->id1)->second;
        tr1::unordered_set<int> *D2 = domains_values.find(pwe->id2)->second;
        int old_size1 = D1->size();
        int old_size2 = D2->size();

        if (refine(graph, D1, D2, pwe, support)) {
            auto iter = arcs.begin();
            for (; iter != arcs.end(); iter++) {
                pair_with_edge *temp_pwe = iter->second;
                delete temp_pwe;
            }
            arcs.clear();
            return;
        }

        //add affected arcs
        NodeX *pNode = p_graph->get_node_with_id(pwe->id1);
        if (old_size1 != D1->size() && pNode->get_edges_size() > 1) {
            int id1 = pwe->id1;
            for (tr1::unordered_map<int, void *>::iterator iter1 = pNode->get_edges_iterator(); iter1 !=
                                                                                                pNode->get_edges_end_iterator(); iter1++) {
                EdgeX *edge = (EdgeX *) (iter1->second);

                int id2 = edge->get_other_node()->get_id();
                if (id2 == pwe->id2)
                    continue;
                string sig = get_sig(id1, id2, pwe->edge_label);

                if (arcs.find(sig) != arcs.end());//delete pwe;
                else {
                    //create pwe
                    pair_with_edge *pwe = new pair_with_edge();
                    pwe->id1 = id1;
                    pwe->id2 = id2;
                    pwe->edge_label = edge->get_label();
                    pwe->min_domain_size = domains_values.find(pwe->id1)->second->size();
                    if (pwe->min_domain_size < domains_values.find(pwe->id2)->second->size())
                        pwe->min_domain_size = domains_values.find(pwe->id2)->second->size();

                    arcs.insert(std::pair<string, pair_with_edge *>(sig, pwe));
                }
            }
        }

        pNode = p_graph->get_node_with_id(pwe->id2);
        if (old_size2 != D2->size() && pNode->get_edges_size() > 1) {
            int id1 = pwe->id2;
            for (tr1::unordered_map<int, void *>::iterator iter1 = pNode->get_edges_iterator(); iter1 !=
                                                                                                pNode->get_edges_end_iterator(); iter1++) {
                EdgeX *edge = (EdgeX *) (iter1->second);

                int id2 = edge->get_other_node()->get_id();
                if (id2 == pwe->id1)
                    continue;
                string sig = get_sig(id1, id2, pwe->edge_label);

                if (arcs.find(sig) != arcs.end());//delete pwe;
                else {
                    //create pwe
                    pair_with_edge *pwe = new pair_with_edge();
                    pwe->id1 = id2;
                    pwe->id2 = id1;
                    pwe->edge_label = edge->get_label();
                    pwe->min_domain_size = domains_values.find(pwe->id1)->second->size();
                    if (pwe->min_domain_size < domains_values.find(pwe->id2)->second->size())
                        pwe->min_domain_size = domains_values.find(pwe->id2)->second->size();

                    arcs.insert(std::pair<string, pair_with_edge *>(sig, pwe));
                }
            }
        }

        arcs.erase(sig);
        delete pwe;
    }

    //clear arcs list
    auto iter = arcs.begin();
    for (; iter != arcs.end(); iter++) {
        pair_with_edge *temp_pwe = iter->second;
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
bool GraMiCounter::refine(GraphX *graph, tr1::unordered_set<int> *D1, tr1::unordered_set<int> *D2, pair_with_edge *pwe,
                          int support) {
    double edgeLabel = pwe->edge_label;
    //set to iterate over the smaller domain
    if (D1->size() > D2->size()) {
        tr1::unordered_set<int> *temp = D1;
        D1 = D2;
        D2 = temp;
    }
    //go over each node in the domain1
    set<int> new_D2;
    for (tr1::unordered_set<int>::iterator iter = D1->begin(); iter != D1->end();) {
        NodeX *node = graph->get_node_with_id((*iter));
        //go over its edges
        bool deleteIt = true;
        for (tr1::unordered_map<int, void *>::iterator iter1 = node->get_edges_iterator(); iter1 !=
                                                                                           node->get_edges_end_iterator(); ++iter1) {
            EdgeX *edge = (EdgeX *) (iter1->second);
            if (edge->get_label() != edgeLabel)
                continue;
            tr1::unordered_set<int>::iterator iter_f = D2->find(edge->get_other_node()->get_id());
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

    for (tr1::unordered_set<int>::iterator iter = D2->begin(); iter != D2->end();) {
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
 * given the graph, check whether it is Acyclic or not (we assume the graph is connected)
 * @param me
 * @return
 */
bool GraMiCounter::is_it_acyclic(GraphX &query) {
    tr1::unordered_set<int> visited;
    vector<int> to_be_visited;

    int current_node_id;
    NodeX *current_node;
    to_be_visited.push_back(0);
    while (visited.size() < query.get_num_of_nodes()) {
        if (to_be_visited.size() == 0)
            break;

        current_node_id = to_be_visited.at(0);
        current_node = query.get_node_with_id(current_node_id);

        to_be_visited.erase(to_be_visited.begin());
        visited.insert(current_node_id);
        //get all neighbor nodes (incoming and outgoing)
        int already_visited_neighbors = 0;//this should not be more than 1

        //all edges!
        for (tr1::unordered_map<int, void *>::iterator iter = current_node->get_edges_iterator(); iter !=
                                                                                                  current_node->get_edges_end_iterator(); iter++) {
            EdgeX *edge = (EdgeX *) iter->second;
            int other_node_id = edge->get_other_node()->get_id();

            if (visited.find(other_node_id) != visited.end()) {
                already_visited_neighbors++;
                if (already_visited_neighbors > 1) {
                    //cout<<"It is CYCLIC!";
                    return false;
                }
            } else {
                to_be_visited.push_back(other_node_id);
            }
        }
    }

    return true;
}

/**
 * Populate the ordering 'order' from domains with smaller sizes to those with larger sizes.、
 * sort the domains according to their size
 */
void GraMiCounter::set_domains_order(tr1::unordered_map<int, tr1::unordered_set<int> *> &domains_values, int *order,
                                     Pattern *pattern) {
    int invalid_col;
    if (Settings::use_predicted_inv_column)
        invalid_col = pattern->get_invalid_col();
    else
        invalid_col = -1;

    tr1::unordered_set<int> added;
    int i = 0;
    // If there is a column that is predicted to be invalid,
    // put it first (first detection, seeking earlier pruning)
    if (invalid_col > -1) {
        order[0] = invalid_col;
        added.insert(invalid_col);
        i = 1;
    }

    for (; i < domains_values.size(); i++) {
        //find the minimum
        int min = std::numeric_limits<int>::max();
        int ID4min = -1;

        for (tr1::unordered_map<int, tr1::unordered_set<int> *>::iterator iter = domains_values.begin();
             iter != domains_values.end(); iter++) {
            //get the pattern node
            int current_id = iter->first;
            tr1::unordered_set<int> *current_set = iter->second;

            // The size of the domain corresponding to the node / the number of edges connected to the node
            long current_domain_value = current_set->size() /
                                        pattern->get_graph()->get_node_with_id(current_id)->get_edges_size();

            if (current_domain_value < min && added.find(current_id) == added.end()) {
                ID4min = current_id;
                min = current_domain_value;
            }
        }

        // Each time the smallest remaining is found, insert
        order[added.size()] = ID4min;
        added.insert(ID4min);
    }

    if (Settings::debug_msg) {
        for (int ii = 0; ii < domains_values.size(); ii++)
            cout << order[ii] << ", ";
        cout << endl;
    }
}
