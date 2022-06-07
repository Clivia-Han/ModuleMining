/**
*
MyMiner.cpp
 */

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include "MyMiner.h"
#include "utils.h"
#include "Settings.h"
#include "GraMiCounter.h"

/**
 * start the mining process, given the filename (file base name for the partitions)
 * , and the required support threshold
 */
void MyMiner::start_mining(string file_name, int graph_type, int support, int given_type) {
    int numberOfCreatedCandids = 0;

    this->support = support;

    //load the graph
    long long start = get_ms_of_day();
    load_graph(file_name, graph_type, support, frequent_edges);

    if (frequent_edges.get_size() == 0) {
        cout << "No frequent patterns found! Exiting" << endl;
        exit(0);
    }

    long long end = get_ms_of_day();
    long long elapsed = end - start;

    cout << "Loading took " << (elapsed / 1000) << " sec and " << (elapsed % 1000) << " ms" << endl;


//    Settings::graphLoadingTime = elapsed;

    CLMap *clmap = new CLMap();

    if (given_type == -1) {
        clmap->add_all(&frequent_edges);
    }
        // Restrict the subgraph must contain nodes of type given_type
    else {
        auto iter = frequent_edges.get_first_element();
        while (iter.pattern != 0) {
            GraphX g = iter.pattern->get_graph();
            if (g.get_node_with_id(0)->get_label() != given_type && g.get_node_with_id(1)->get_label() != given_type) {
                frequent_edges.remove_pattern(iter.pattern);
            }
            frequent_edges.advance_iterator(iter);
        }
    }

    frequent_patterns.insert(frequent_patterns.begin() + 1, clmap);//frequent edges!

    if (Settings::debug_msg) {
        cout << "#frequent edges  = " << frequent_edges.get_size() << endl;
        cout << "Frequent edges:" << endl;
        frequent_edges.print();
    }

    //extend frequent edges into 2 edges graphs

    start = get_ms_of_day();

    extend_freq_edges();

    end = get_ms_of_day();

    cout << "Start mining ..." << endl;

    tr1::unordered_set<int> delete_pattern_id = tr1::unordered_set<int>();

    while (true) {
        //check if we have more candidates with the same current size
        if (currently_checking.size() == 0 && get_num_elems((vector<map<string, void *> *> *) &candidates) == 0) {
            break;
        }

        //send available candidates to frequency counting
        bool is_freq = false;
        domains_solutions.clear();
        char can_info[50];
        strcpy(can_info, pop_my_candidate(candidates, currently_checking, support, -1));

        int res, candidate_id = 0;
        sscanf(can_info, "%d,%d", &res, &candidate_id);
        if (res == 1) {
//            for (auto domain_solution : domains_solutions) {
//                cout << domain_solution.second.size() <<endl;
//            }
            is_freq = true;
            if (Settings::debug_msg) {
                cout << "frequent!" << endl;
                cout << can_info << endl;
            }
        }

        if (is_freq) {
            Pattern *candidate = currently_checking.find(candidate_id)->second;
            candidate->set_frequency(support);
            currently_checking.erase(candidate_id);
            if ((frequent_patterns.size() - 1) < candidate->get_size()) {
                frequent_patterns.insert(frequent_patterns.begin() + candidate->get_size(), new CLMap());
            }
            frequent_patterns[candidate->get_size()]->add_pattern(candidate);
            frequent_pattern_vec.emplace_back(candidate);
//            frequent_patterns_domain[frequent_pattern_vec.size()] = candidate->get_domain_values();

            if (Settings::debug_msg) {
                cout << "insert into frequent_patterns_domain" << endl;
                cout << "count: " << frequent_pattern_vec.size() - 1 << endl;
                for (auto &domain: domains_solutions) {
                    cout << "domain ID: " << domain.first << endl;
                    for (auto &value: domain.second) {
                        cout << "value ID: " << value << endl;
                    }
                }
            }

            frequent_patterns_domain[frequent_pattern_vec.size() - 1] = domains_solutions;
//            frequent_patterns_domain.insert(std::pair<int, map<int, set<int>>>(frequent_pattern_vec.size(), domains_solutions));
            if (Settings::debug_msg)
                cout << "size of frequent_patterns_domain: " << frequent_patterns_domain.size() << endl;

            //extend using the current frequent patterns with the same size
            if (Settings::debug_msg)
                cout << "extending based on current frequent pattern" << endl;
            CLMap_Iterator iter1 = frequent_patterns[candidate->get_size()]->get_first_element();
            while (iter1.pattern != 0) {
                Pattern *temp = iter1.pattern;//->second;
                frequent_patterns[candidate->get_size()]->advance_iterator(iter1);

                set<std::pair<PrimaryGraph *, PrimaryGraph *> > joining_pgs = candidate->get_joining_pg(temp);
                for (const auto &joining_pg: joining_pgs) {
                    PrimaryGraph *cand_pg = joining_pg.first;
                    PrimaryGraph *other_pg = joining_pg.second;

                    //find isomorphisms
                    vector<map<int, int> *> result;
                    tr1::unordered_map<int, tr1::unordered_set<int> *> domains_values;
                    other_pg->get_graph()->is_isomorphic(cand_pg->get_graph(), result, domains_values);

                    //for each result
                    for (auto &iter2: result) {
                        map<int, int> *curr_result = iter2;

                        //add the new edge
                        int scr_node_id = curr_result->find(other_pg->get_src_node_id())->second;
                        EdgeX *edge = other_pg->get_edge();
                        GraphX *new_graph = new GraphX(candidate->get_graph());
                        int new_id = new_graph->get_num_of_nodes();
                        while (new_graph->get_node_with_id(new_id) != NULL) {
                            new_id++;
                        }
                        new_graph->add_node(new_id, edge->get_other_node()->get_label());
                        new_graph->add_edge(scr_node_id, new_id, edge->get_label());
                        //create the new candidate object
                        Pattern *new_candidate = new Pattern(new_graph, false);
                        if (candidates.size() <= new_candidate->get_size()) {
                            candidates.insert(candidates.begin() + new_candidate->get_size(), new CLMap());
                        }
                        new_candidate->generate_primary_graphs();
                        bool b = candidates.at(new_candidate->get_size())->add_pattern(new_candidate);
                        if (!b) delete new_candidate;
                        //if the two edges have the same label, we need to assume the same label belong to one node
                        if (cand_pg->get_edge()->get_other_node()->get_label() ==
                            other_pg->get_edge()->get_other_node()->get_label() &&
                            cand_pg->get_src_node_id() != scr_node_id) {
                            new_graph = new GraphX(candidate->get_graph());
                            new_graph->add_edge(cand_pg->get_edge()->get_other_node()->get_id(), scr_node_id,
                                                other_pg->get_edge()->get_label());
                            new_candidate = new Pattern(new_graph, false);
                            new_candidate->generate_primary_graphs();
                            //CHECK THIS LINE IS TAKING SO MUCH TIME WHEN THE PATTERN IS BIG!!!! ---->
                            bool b = candidates.at(new_candidate->get_size())->add_pattern(new_candidate);
                            if (!b) delete new_candidate;
                        }
                        delete iter2;
                    }
                }
            }
        } else {
            //cout << "find candidate and remove" << endl;
            Pattern *candidate = currently_checking.find(candidate_id)->second;
            candidate->set_frequency(0);
            //cout << "erase" << endl;
            currently_checking.erase(candidate_id);
        }
    }

    if (Settings::debug_msg) {
        cout << "size of ultimate frequent_patterns_domain: " << frequent_patterns_domain.size() << endl;
        cout << "now print all frequent_patterns_domain!" << endl;
        cout << "test1" << endl;
        print_domains();
//        for (auto &pattern_iter: frequent_patterns_domain) {
//            cout << *(frequent_pattern_vec[pattern_iter.first]->get_graph()) << endl;
//            for (auto &map_iter: pattern_iter.second) {
//                cout << "domain: " << map_iter.first << endl;
//                cout << "values: " << endl;
//                set<int> st = map_iter.second;
//                for (auto &value_iter: st) {
//                    cout << value_iter << endl;
//                }
//            }
//        }
        cout << "remove partial graph" << endl;
    }

//    now delete invalid frePatterns
    int pattern_cnt = 0;
    for (auto fre_pattern: frequent_pattern_vec) {
        if (Settings::debug_msg) {
            cout << "test this subgraph!" << endl;
            cout << *(fre_pattern->get_graph()) << endl;
        }
        bool del = false;
        print_domains();
        auto domain_nodes = frequent_patterns_domain[pattern_cnt];
        if (!domain_nodes.empty()) {
            if (Settings::debug_msg) {
                cout << "subgraph found!";
            }
        } else {
            cout << "empty domain_nodes" << pattern_cnt << endl;
        }

        if (Settings::debug_msg) {
            for (auto &domain_node: domain_nodes) {
                int value_num = 0;
                cout << "domain ID: " << domain_node.first << endl;
                for (int value_iter: domain_node.second) {
                    cout << "value ID" << value_num << ": " << value_iter << endl;
                    value_num++;
                }
            }
        }

        GraphX *pg = fre_pattern->get_graph();
        for (auto &node_iter1: domain_nodes) {
            NodeX *p_node = fre_pattern->get_graph()->get_node_with_id(node_iter1.first);
            set<int> node_set = node_iter1.second;
            vector<int> linked_domain;
            vector<int> other_domain;
            // 得到pNode的邻接结点linkedDomain
            for (auto edge_iter1 = p_node->get_edges_iterator();
                 edge_iter1 != p_node->get_edges_end_iterator(); ++edge_iter1) {
                // linked_id: the other nodeID of the edge
                int linked_id = edge_iter1->first;
                linked_domain.push_back(linked_id);
            }
            // 不与pNode邻接的结点otherDomain
            for (auto node_iter2 = pg->get_nodes_iterator(); node_iter2 != pg->get_nodes_end_iterator(); ++node_iter2) {
                if (node_iter2->second->get_id() == p_node->get_id()) {
                    continue;
                }
                bool flag = false;
                for (int i: linked_domain) {
                    if (node_iter2->second->get_id() == i) {
                        flag = true;
                        break;
                    }
                }
                if (!flag) {
                    other_domain.push_back(node_iter2->second->get_id());
                }
            }

            int num1 = 0;
            if (Settings::debug_msg) {
                for (int &value_iter: linked_domain) {
                    cout << "linked value ID" << num1 << ": " << value_iter << endl;
                    num1++;
                }
                int num2 = 0;
                for (int &value_iter: other_domain) {
                    cout << "other value ID" << num2 << ": " << value_iter << endl;
                    num2++;
                }
            }

            for (auto id: node_set) {
                // 遍历pNode的域，当前结点为gNode
                NodeX *g_node = graph->get_node_with_id(id);
                if (g_node == nullptr) {
                    cout << "g_node not found!" << endl;
                    exit(0);
                } else {
                    if (Settings::debug_msg) {
                        cout << "find g_node!" << endl;
                    }
                }
                for (tr1::unordered_map<int, void *>::iterator edge_iter2 = g_node->get_edges_iterator();
                     edge_iter2 != g_node->get_edges_end_iterator(); ++edge_iter2) {
                    // gNode的邻接结点otherNodeID
                    int other_node_id = edge_iter2->first;
                    bool flag1 = false;
                    for (int k: linked_domain) {
                        auto linked_values = domain_nodes[k];
                        if (linked_values.empty()) {
                            cout << "linked_domain[k]" << k << " not found in domain_nodes!";
                            exit(0);
                        } else {
                            if (Settings::debug_msg) {
                                cout << "linked_domain[k]" << k << " found in domain_nodes!";
                            }
                        }
                        // 与当前gNode相连的节点不在pNode的相邻节点的域中
                        if (linked_values.find(other_node_id) != linked_values.end()) {
                            // 当前的otherNode通过审核
                            break;
                        }
                        for (int j: other_domain) {
                            auto other_values = domain_nodes[j];
                            if (other_values.empty()) {
                                cout << "other_values[k] not found in other_values!";
                                exit(0);
                            } else {
                                if (Settings::debug_msg) {
                                    cout << "other_values[k] found in other_values!";
                                }
                            }
//                                set<int>* other_values = domain_nodes[other_domain[j]];
                            // 与当前gNode相连的节点却存在于pattern中其他结点的域中，说明当前的pattern中结点的联系没有被完全表示，不符合module的要求
                            if (other_values.find(other_node_id) != other_values.end()) {
                                flag1 = true;
                                break;
                            }
                        }
                        if (flag1) {
                            del = true;
                            break;
                        }
                    }
                    if (del) {
                        break;
                    }
                }
                if (del) {
                    break;
                }
            }
            if (del) {
                break;
            }
        }
        if (del) {
//            frequent_patterns[fre_pattern->get_size()]->remove_pattern(fre_pattern);
//            frequent_pattern_vec.erase(pattern_iter);
            if (Settings::debug_msg) {
                cout << "delete this graph" << endl;
                cout << *(fre_pattern->get_graph()) << endl;
            }
            delete_pattern_id.insert(pattern_cnt);
        }
        pattern_cnt++;
    }
    print_result(delete_pattern_id);
}

char *MyMiner::pop_my_candidate(vector<CLMap *> &candidates, map<int, Pattern *> &currently_checking, int support,
                                double approximate) {
    if (Settings::debug_msg)
        cout << "try to pop a candidate ..." << endl;
    static char result[50];
    bool is_freq = false;
    CLMap *current_candidates = 0;
    for (auto &candidate: candidates) {
        if (candidate->get_size() != 0) {
            current_candidates = candidate;
            break;
        }
    }

    if (current_candidates == 0) {
        if (Settings::debug_msg)
            cout << "No more candidate ..." << endl;
        result[0] = '0';
        return result;
    }

    if (Settings::debug_msg)
        cout << "Popping a candidate ..." << endl;
    //get a candidate, if there is
    if (current_candidates->get_size() > 0) {
        CLMap_Iterator iter = current_candidates->get_first_element();
        string key = iter.key;
        Pattern *candidate = iter.pattern;
        current_candidates->remove_pattern(candidate);

        // sendACandidateApprox(key, candidate, currently_checking, destination);
        if (currently_checking.find(candidate->get_id()) != currently_checking.end()) {
            cout << "candidate->get_id() = " << candidate->get_id() << endl << flush;
            exit(0);
        }

        if (Settings::debug_msg) {
            cout << "candidate ID: " << candidate->get_id() << endl;
        }
        currently_checking[candidate->get_id()] = candidate;
//		currently_checking.insert(std::pair<int, Pattern*>(candidate->get_id(), candidate));

        Miner::num_is_freq_calls++;

        ostringstream temp_os;
        temp_os << *(candidate->get_graph());
        char graph_str[2000];
        sprintf(graph_str, "%d,%s\t", candidate->get_id(), temp_os.str().c_str());
        if (Settings::debug_msg)
            cout << "now pattern: " << graph_str << endl;

        is_freq = work_count(graph_str, support, approximate);
        int fre = is_freq ? 1 : 0;
        sprintf(result, "%d,%d", fre, candidate->get_id());
    }

    return result;
}

void clear_all_mp(tr1::unordered_map<string, void *> &all_mp) {
    for (tr1::unordered_map<string, void *>::iterator iter = all_mp.begin(); iter != all_mp.end(); iter++) {
        delete iter->second;
    }
    all_mp.clear();
}

bool MyMiner::work_count(char *graph_str, int support, double approximate) {
    tr1::unordered_map<string, void *> all_mps;
    GraphX *subgraph = new GraphX(1, false);

    int candidate_id;
    char subgraph_temp[500];
    sscanf(graph_str, "%d,%[^\t]", &candidate_id, subgraph_temp);
    std::stringstream ss;
    ss << subgraph_temp;
    string subgraph_str = ss.str();
    subgraph->load_from_string(subgraph_str, all_mps);
    clear_all_mp(all_mps);
    Pattern *new_candidate = new Pattern(subgraph, false);

    unsigned long start_time = get_ms_of_day();
    int freq = get_freq(new_candidate, support, approximate);
    bool is_freq;
    if (freq >= support) {
        is_freq = true;
//        cout << "test in fuc work_count" << endl;
//        cout << *subgraph;
//        for (auto & domains_solution : domains_solutions) {
//            cout << "domain ID: " << domains_solution.first << "domain size: " << domains_solution.second.size() << endl;
//            for (auto & value_iter : domains_solution.second) {
//                cout << "value ID: " << value_iter;
//            }
//        }
    } else
        is_freq = false;

    unsigned long elapsed = get_ms_of_day() - start_time;

    if (Settings::debug_msg)
        printf("Time taken is: %Lu. Predicted total time is: %lu\n", elapsed);

    delete subgraph;
    delete new_candidate;
    if (is_freq && Settings::debug_msg) {
        cout << "work_count True!" << endl;
    }
    return is_freq;
}


void MyMiner::print_total_expected_time() {
    vector<CLMap *> *all_patterns[2];
    all_patterns[0] = &frequent_patterns;
    all_patterns[1] = &infrequent_patterns;

    unsigned long int totat_time = 0;

    //add to candidates
    for (int l = 0; l < 2; l++) {
        for (vector<CLMap *>::iterator iter = all_patterns[l]->begin(); iter != all_patterns[l]->end(); iter++) {
            CLMap *temp_map = (*iter);
            CLMap_Iterator iter1 = temp_map->get_first_element();
            while (iter1.pattern != 0) {
                Pattern *pattern = iter1.pattern;//->second;
                temp_map->advance_iterator(iter1);
                totat_time += pattern->get_predicted_time();
            }
        }
    }
}

int MyMiner::get_freq(Pattern *candidate, int support, double approximate) {
    //cout << "now counting freq!" << endl;
    vector<unsigned long> list_of_num_of_iters;
    int freq = 0;
    freq = GraMiCounter::is_frequent(graph, candidate, support, approximate, domains_solutions);
    if (Settings::debug_msg) {
        if (freq >= support) {
            cout << *candidate->get_graph();
            for (auto &domains_solution: domains_solutions) {
                cout << "domain ID: " << domains_solution.first << "domain size: " << domains_solution.second.size()
                     << endl;
                for (auto &value_iter: domains_solution.second) {
                    cout << "value ID: " << value_iter;
                }
            }
        }
    }
    return freq;
}


void MyMiner::print_result(tr1::unordered_set<int> delete_pattern_id) {
    cout << "print final result!";
    int cnt = 0;
    for (auto &pattern: frequent_pattern_vec) {
        if (delete_pattern_id.find(cnt) != delete_pattern_id.end()) {
            cnt++;
            continue;
        }
        cout << endl;
        cout << "*****frequent pattern " << cnt << "*****" << endl;
        cout << *(frequent_pattern_vec[cnt]->get_graph()) << endl;
        auto now_domain = frequent_patterns_domain[cnt];
        for (auto &map_iter: now_domain) {
            cout << "domain: " << map_iter.first << endl;
            cout << "values: " << endl;
            set<int> st = map_iter.second;
            for (auto &value_iter: st) {
                cout << value_iter << endl;
            }
        }
        cout << "**************************" << endl;
        cnt++;
    }
}

void MyMiner::print_domains() {
    for (auto &pattern_iter: frequent_patterns_domain) {
        cout << "subgraph" << pattern_iter.first << ":" << endl;
        cout << *(frequent_pattern_vec[pattern_iter.first]->get_graph()) << endl;
        for (auto &map_iter: pattern_iter.second) {
            cout << "domain: " << map_iter.first << endl;
            cout << "values: " << endl;
            set<int> st = map_iter.second;
            for (auto &value_iter: st) {
                cout << value_iter << endl;
            }
        }
    }
}
