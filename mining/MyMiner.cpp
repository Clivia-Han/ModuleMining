/**
*
MyMiner.cpp
 */

#include "MyMiner.h"
#include "Settings.h"
#include "core_file.h"
#include "GraMiCounter.h"
#include "mining_utils.h"

/**
 * start the mining process, given the filename (file base name for the partitions)
 * , and the required support threshold
 */
void MyMiner::start_mining(std::string file_name, int graph_type, int support, std::string given_type) {
    int numberOfCreatedCandids = 0;

    this->support = support;

    //load the graph
    long long start = get_ms_of_day();
    load_graph(file_name, graph_type, support, frequent_edges);

    if (frequent_edges.get_size() == 0) {
        std::cout << "No frequent patterns found! Exiting" << std::endl;
        exit(0);
    }

    long long end = get_ms_of_day();
    long long elapsed = end - start;

    std::cout << "Loading took " << (elapsed / 1000) << " sec and " << (elapsed % 1000) << " ms" << std::endl;


//    Settings::graphLoadingTime = elapsed;

    CLMap *clmap = new CLMap();

    if (stoi(given_type) == -1) {
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
        std::cout << "#frequent edges  = " << frequent_edges.get_size() << std::endl;
        std::cout << "Frequent edges:" << std::endl;
        frequent_edges.print();
    }

    //extend frequent edges into 2 edges graphs

    start = get_ms_of_day();

    extend_freq_edges();

    end = get_ms_of_day();

    std::cout << "Start mining ..." << std::endl;

    std::tr1::unordered_set<int> delete_pattern_id = std::tr1::unordered_set<int>();

    while (true) {
        //check if we have more candidates with the same current size
        if (currently_checking.size() == 0 && get_num_elems((std::vector<std::map<std::string, void *> *> *) &candidates) == 0) {
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
//                std::cout << domain_solution.second.size() <<std::endl;
//            }
            is_freq = true;
            if (Settings::debug_msg) {
                std::cout << "frequent!" << std::endl;
                std::cout << can_info << std::endl;
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
                std::cout << "insert into frequent_patterns_domain" << std::endl;
                std::cout << "count: " << frequent_pattern_vec.size() - 1 << std::endl;
                for (auto &domain: domains_solutions) {
                    std::cout << "domain ID: " << domain.first << std::endl;
                    for (auto &value: domain.second) {
                        std::cout << "value ID: " << value << std::endl;
                    }
                }
            }

            frequent_patterns_domain[frequent_pattern_vec.size() - 1] = domains_solutions;
//            frequent_patterns_domain.insert(std::pair<int, std::map<int, set<int>>>(frequent_pattern_vec.size(), domains_solutions));
            if (Settings::debug_msg)
                std::cout << "size of frequent_patterns_domain: " << frequent_patterns_domain.size() << std::endl;

            //extend using the current frequent patterns with the same size
            if (Settings::debug_msg)
                std::cout << "extending based on current frequent pattern" << std::endl;
            CLMap_Iterator iter1 = frequent_patterns[candidate->get_size()]->get_first_element();
            while (iter1.pattern != 0) {
                Pattern *temp = iter1.pattern;//->second;
                frequent_patterns[candidate->get_size()]->advance_iterator(iter1);

                std::set<std::pair<PrimaryGraph *, PrimaryGraph *> > joining_pgs = candidate->get_joining_pg(temp);
                for (const auto &joining_pg: joining_pgs) {
                    PrimaryGraph *cand_pg = joining_pg.first;
                    PrimaryGraph *other_pg = joining_pg.second;

                    //find isomorphisms
                    std::vector<std::map<int, int> *> result;
                    std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> domains_values;
                    other_pg->get_graph()->is_isomorphic(cand_pg->get_graph(), result, domains_values);

                    //for each result
                    for (auto &iter2: result) {
                        std::map<int, int> *curr_result = iter2;

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
            //std::cout << "find candidate and remove" << std::endl;
            Pattern *candidate = currently_checking.find(candidate_id)->second;
            candidate->set_frequency(0);
            //std::cout << "erase" << std::endl;
            currently_checking.erase(candidate_id);
        }
    }

    if (Settings::debug_msg) {
        std::cout << "size of ultimate frequent_patterns_domain: " << frequent_patterns_domain.size() << std::endl;
        std::cout << "now print all frequent_patterns_domain!" << std::endl;
        print_domains();
//        for (auto &pattern_iter: frequent_patterns_domain) {
//            std::cout << *(frequent_pattern_vec[pattern_iter.first]->get_graph()) << std::endl;
//            for (auto &map_iter: pattern_iter.second) {
//                std::cout << "domain: " << map_iter.first << std::endl;
//                std::cout << "values: " << std::endl;
//                set<int> st = map_iter.second;
//                for (auto &value_iter: st) {
//                    std::cout << value_iter << std::endl;
//                }
//            }
//        }
        std::cout << "remove partial graph" << std::endl;
    }

//    now delete invalid frePatterns
    int pattern_cnt = 0;
    for (auto fre_pattern: frequent_pattern_vec) {
        if (Settings::debug_msg) {
            std::cout << "test this subgraph!" << std::endl;
            std::cout << *(fre_pattern->get_graph()) << std::endl;
        }
        bool del = false;
        print_domains();
        auto domain_nodes = frequent_patterns_domain[pattern_cnt];
        if (!domain_nodes.empty()) {
            if (Settings::debug_msg) {
                std::cout << "subgraph found!";
            }
        } else {
            std::cout << "empty domain_nodes" << pattern_cnt << std::endl;
        }

        if (Settings::debug_msg) {
            for (auto &domain_node: domain_nodes) {
                int value_num = 0;
                std::cout << "domain ID: " << domain_node.first << std::endl;
                for (int value_iter: domain_node.second) {
                    std::cout << "value ID" << value_num << ": " << value_iter << std::endl;
                    value_num++;
                }
            }
        }

        GraphX *pg = fre_pattern->get_graph();
        for (auto &node_iter1: domain_nodes) {
            NodeX *p_node = fre_pattern->get_graph()->get_node_with_id(node_iter1.first);
            std::set<int> node_set = node_iter1.second;
            std::vector<int> linked_domain;
            std::vector<int> other_domain;
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
                    std::cout << "linked value ID" << num1 << ": " << value_iter << std::endl;
                    num1++;
                }
                int num2 = 0;
                for (int &value_iter: other_domain) {
                    std::cout << "other value ID" << num2 << ": " << value_iter << std::endl;
                    num2++;
                }
            }

            for (auto id: node_set) {
                // 遍历pNode的域，当前结点为gNode
                NodeX *g_node = graph->get_node_with_id(id);
                if (g_node == nullptr) {
                    std::cout << "g_node not found!" << std::endl;
                    exit(0);
                } else {
                    if (Settings::debug_msg) {
                        std::cout << "find g_node!" << std::endl;
                    }
                }
                for (std::tr1::unordered_map<int, void *>::iterator edge_iter2 = g_node->get_edges_iterator();
                     edge_iter2 != g_node->get_edges_end_iterator(); ++edge_iter2) {
                    // gNode的邻接结点otherNodeID
                    int other_node_id = edge_iter2->first;
                    bool flag1 = false;
                    for (int k: linked_domain) {
                        auto linked_values = domain_nodes[k];
                        if (linked_values.empty()) {
                            std::cout << "linked_domain[k]" << k << " not found in domain_nodes!";
                            exit(0);
                        } else {
                            if (Settings::debug_msg) {
                                std::cout << "linked_domain[k]" << k << " found in domain_nodes!";
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
                                std::cout << "other_values[k] not found in other_values!";
                                exit(0);
                            } else {
                                if (Settings::debug_msg) {
                                    std::cout << "other_values[k] found in other_values!";
                                }
                            }
//                                std::set<int>* other_values = domain_nodes[other_domain[j]];
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
                std::cout << "delete this graph" << std::endl;
                std::cout << *(fre_pattern->get_graph()) << std::endl;
            }
            delete_pattern_id.insert(pattern_cnt);
        }
        pattern_cnt++;
    }
    print_result(delete_pattern_id);
}

void MyMiner::generate_now_graph(System &sym, int graph_type, int support, int seed_node_id) {
    graph = new GraphX(1, graph_type);
    graph->set_frequency(support);
    bool first_edge_met = false;

    std::tr1::unordered_map<std::string, void *> edge_to_freq;
    std::map<std::string, std::map<int, std::set<int>>> edge_pairs;

    for (int node_id: sym.now_graph_.node_id_set) {
        std::vector<int> node_attr = sym.node_attributes(node_id);
        std::string node_label = sym.generate_label(node_attr);
        graph->add_node(node_id, node_label);
    }

    for (int edge_id: sym.now_graph_.edge_id_set) {
        std::vector<int> edge_attr = sym.edge_attributes(edge_id);
        std::string edge_label = sym.generate_label(edge_attr);
        if (!first_edge_met) {
            first_edge_met = true;
            if (graph->get_frequency() > -1) {
                std::tr1::unordered_map<std::string, std::set<int> *> nodes_by_label = graph->get_nodes_by_label();
                for (std::tr1::unordered_map<std::string, std::set<int> *>::iterator iter = nodes_by_label.begin();
                     iter != nodes_by_label.end(); iter++) {
                    if (iter->second->size() < graph->get_frequency()) {
                        //如果带某标签的节点在图中出现的次数小于阈值，也就意味着与带有该标签的节点相连的所有边都是不频繁的
                        std::tr1::unordered_map<std::string, std::set<int> *>::iterator iter2 = nodes_by_label.find(
                                iter->first);
                        if (iter2 == nodes_by_label.end()) {
                            std::cout << "Error: isufnm44" << std::endl;
                            exit(0);
                        }
                        //遍历所有带该标签的节点，iter2为其中的节点，忽略此节点与和此节点相连的边
                        std::set<int> *nodes_to_remove = iter2->second;
                        for (std::set<int>::iterator iter1 = nodes_to_remove->begin();
                             iter1 != nodes_to_remove->end();) {
                            int nodeID = *iter1;
                            iter1++;
                            graph->remove_node_ignore_edges(nodeID);
                        }
                    }
                }
            }
        }
        int id1 = sym.storage_.edges_[edge_id].source_;
        int id2 = sym.storage_.edges_[edge_id].target_;
        graph->add_edge(id1, id2, edge_label, edge_to_freq, edge_pairs);
    }

    std::cout << "Master: loop starts" << std::endl;
    //遍历所有边，将频繁边加入frefqEdges
    for (std::tr1::unordered_map<std::string, void *>::iterator iter = edge_to_freq.begin(); iter != edge_to_freq.end(); ++iter) {
        if (((Pattern *) ((*iter).second))->get_frequency() >= support) {
            Pattern *p = (Pattern *) ((*iter).second);
            std::cout << *(p->get_graph()) << std::endl;

            std::string key = iter->first;
            bool flag = false;

            for (auto &edge_pair : edge_pairs[key]) {
                if (edge_pair.first == seed_node_id && !flag)
                    flag = true;
                if (freq_edge_pairs.find(edge_pair.first) == freq_edge_pairs.end()) {
                    freq_edge_pairs[edge_pair.first] = std::set<int>();
                }
                for (auto &x : edge_pair.second) {
                    if (x == seed_node_id && !flag)
                        flag = true;
                    freq_edge_pairs[edge_pair.first].insert(x);
                }
            }

            std::string now_cl = p->get_graph()->get_canonical_label();

//            freq_pattern_pairs[key] = freq_edge_pairs;
            if (cl_set.count(now_cl) < 1) {
                if (seed_node_id != -1 && !flag)
                    continue;
                cl_set.insert(now_cl);
                frequent_edges.add_pattern(p);
                frequent_pattern_vec.push_back(p);
                frequent_patterns_domain.insert(std::pair<int, std::map<int, std::set<int>>>(frequent_pattern_vec.size() - 1,
                                                                                   p->get_domain_values()));
            }
        } else
            delete (Pattern *) ((*iter).second);
    }
    edge_pairs.clear();
    std::cout << "Master: loop finishes" << std::endl;

    std::cout << " data loaded successfully!" << std::endl;
}

/**
 * start the mining process, given the filename (file base name for the partitions)
 * , and the required support threshold
 */
void MyMiner::start_mining_module(System &sym, int graph_type, int support, int seed_node_id, const std::string& path) {
    this->support = support;

    //load the graph
    long long start = get_ms_of_day();
    generate_now_graph(sym, graph_type, support, seed_node_id);

    if (frequent_edges.get_size() == 0) {
        std::cout << "No frequent patterns found! Exiting" << std::endl;
        exit(0);
    }

    long long end = get_ms_of_day();
    long long elapsed = end - start;

    std::cout << "Loading took " << (elapsed / 1000) << " sec and " << (elapsed % 1000) << " ms" << std::endl;


//    Settings::graphLoadingTime = elapsed;

    CLMap *clmap = new CLMap();

    clmap->add_all(&frequent_edges);

    CLMap *empty_clmap = new CLMap();
    frequent_patterns.insert(frequent_patterns.begin() + 1, empty_clmap);
    frequent_patterns.insert(frequent_patterns.begin() + 2, clmap);//frequent edges!

    if (Settings::debug_msg) {
        std::cout << "#frequent edges  = " << frequent_edges.get_size() << std::endl;
        std::cout << "Frequent edges:" << std::endl;
        frequent_edges.print();

        std::cout << "print freq_edge_pairs" << std::endl;
        for (auto &freq_edge_iter : freq_edge_pairs) {
            int id0 = freq_edge_iter.first;
            std::cout << "first node: " << id0 << std::endl;
            for (auto &id1:freq_edge_iter.second) {
                std::cout << "second node: " << id1 << std::endl;
            }
            std::cout << std::endl;
        }
    }

    if (seed_node_id == -1) {
        for (auto &freq_edge_iter : freq_edge_pairs) {
            int id0 = freq_edge_iter.first;
            for (auto &other_iter : freq_edge_iter.second) {
                int id1 = other_iter;
                id_set = std::set<int>();
                id_map = std::map<int, int>();
                id_set.insert(id0);
                id_set.insert(id1);
                id_map[id0] = 0;
                id_map[id1] = 1;
                std::string el = graph->get_edge_label(id0, id1);
                std::string nl0 = graph->get_node_with_id(id0)->get_label();
                std::string nl1 = graph->get_node_with_id(id1)->get_label();
                now_graph = new GraphX(1, graph_type);
//            GraphX *pre_graph = new GraphX(now_graph);
                now_graph->add_node(0, nl0);
                now_graph->add_node(1, nl1);
                now_graph->add_edge(0, 1, el);
                //if (Settings::debug_msg) {
                //    std::cout << "test now_graph" << std::endl;
                //    std::cout << *(now_graph) << std::endl;
                //}
                for (auto &old_id : id_set) {
                    for (auto &new_id: freq_edge_pairs[old_id]) {
                        if (id_set.find(new_id) != id_set.end()) {
                            continue;
                        }
                        DFS(old_id, new_id);
                        for (auto &pid : id_set) {
                            if (freq_edge_pairs[pid].find(new_id) != freq_edge_pairs[pid].end()) {
                                now_graph->remove_edge(id_map[pid], id_map[new_id]);
                            }
                        }
                        //if (Settings::debug_msg) {
                        //    std::cout << "roll back" << std::endl;
                        //    std::cout << *(now_graph) << std::endl;
                        //}
                        id_set.erase(new_id);
                        id_map.erase(new_id);
                    }
                }
            }
        }
    } else {
        for (auto &other_iter : freq_edge_pairs[seed_node_id]) {
            int other_id = other_iter;
            std::cout << "other id: " << seed_node_id << std::endl;
            std::cout << "other id: " << other_id << std::endl;
            id_set = std::set<int>();
            id_map = std::map<int, int>();
            id_set.insert(seed_node_id);
            id_set.insert(other_id);
            id_map[seed_node_id] = 0;
            id_map[other_id] = 1;
            std::string el = graph->get_edge_label(seed_node_id, other_id);
            std::string nl0 = graph->get_node_with_id(seed_node_id)->get_label();
            std::string nl1 = graph->get_node_with_id(other_id)->get_label();
            now_graph = new GraphX(1, graph_type);
//            GraphX *pre_graph = new GraphX(now_graph);
            now_graph->add_node(0, nl0);
            now_graph->add_node(1, nl1);
            now_graph->add_edge(0, 1, el);
            //if (Settings::debug_msg) {
            //    std::cout << "test now_graph" << std::endl;
            //    std::cout << *(now_graph) << std::endl;
            //}
            for (auto &old_id : id_set) {
                for (auto &new_id: freq_edge_pairs[old_id]) {
                    if (id_set.find(new_id) != id_set.end()) {
                        continue;
                    }
                    DFS(old_id, new_id);
                    for (auto &pid : id_set) {
                        if (freq_edge_pairs[pid].find(new_id) != freq_edge_pairs[pid].end()) {
                            now_graph->remove_edge(id_map[pid], id_map[new_id]);
                        }
                    }
                    //if (Settings::debug_msg) {
                    //    std::cout << "roll back" << std::endl;
                    //    std::cout << *(now_graph) << std::endl;
                    //}
                    id_set.erase(new_id);
                    id_map.erase(new_id);
                }
            }
        }
    }
    for (auto &frequent_pattern : frequent_pattern_vec) {
        std::cout << *(frequent_pattern->get_graph()) << std::endl;
    }
    print_frequent_module();
    write_solutions(path);
}

/**
 * start the mining process, given the filename (file base name for the partitions)
 * , and the required support threshold
 */
void MyMiner::start_mining_module(std::string file_name, int graph_type, int support, int seed_node_id) {
    this->support = support;

    //load the graph
    long long start = get_ms_of_day();
    load_graph_with_pairs(file_name, graph_type, support, frequent_edges, seed_node_id);

    if (frequent_edges.get_size() == 0) {
        std::cout << "No frequent patterns found! Exiting" << std::endl;
        exit(0);
    }

    long long end = get_ms_of_day();
    long long elapsed = end - start;

    std::cout << "Loading took " << (elapsed / 1000) << " sec and " << (elapsed % 1000) << " ms" << std::endl;


//    Settings::graphLoadingTime = elapsed;

    CLMap *clmap = new CLMap();

    clmap->add_all(&frequent_edges);

    frequent_patterns.insert(frequent_patterns.begin() + 1, clmap);//frequent edges!

    if (Settings::debug_msg) {
        std::cout << "#frequent edges  = " << frequent_edges.get_size() << std::endl;
        std::cout << "Frequent edges:" << std::endl;
        frequent_edges.print();

        std::cout << "print freq_edge_pairs" << std::endl;
        for (auto &freq_edge_iter : freq_edge_pairs) {
            int id0 = freq_edge_iter.first;
            std::cout << "first node: " << id0 << std::endl;
            for (auto &id1:freq_edge_iter.second) {
                std::cout << "second node: " << id0 << std::endl;
            }
            std::cout << std::endl;
        }
    }

    if (seed_node_id == -1) {
        for (auto &freq_edge_iter : freq_edge_pairs) {
            int id0 = freq_edge_iter.first;
            for (auto &other_iter : freq_edge_iter.second) {
                int id1 = other_iter;
                id_set = std::set<int>();
                id_map = std::map<int, int>();
                id_set.insert(id0);
                id_set.insert(id1);
                id_map[id0] = 0;
                id_map[id1] = 1;
                std::string el = graph->get_edge_label(id0, id1);
                std::string nl0 = graph->get_node_with_id(id0)->get_label();
                std::string nl1 = graph->get_node_with_id(id1)->get_label();
                now_graph = new GraphX(1, graph_type);
//            GraphX *pre_graph = new GraphX(now_graph);
                now_graph->add_node(0, nl0);
                now_graph->add_node(1, nl1);
                now_graph->add_edge(0, 1, el);
                //if (Settings::debug_msg) {
                //    std::cout << "test now_graph" << std::endl;
                //    std::cout << *(now_graph) << std::endl;
                //}
                for (auto &old_id : id_set) {
                    for (auto &new_id: freq_edge_pairs[old_id]) {
                        if (id_set.find(new_id) != id_set.end()) {
                            continue;
                        }
                        DFS(old_id, new_id);
                        for (auto &pid : id_set) {
                            if (freq_edge_pairs[pid].find(new_id) != freq_edge_pairs[pid].end()) {
                                now_graph->remove_edge(id_map[pid], id_map[new_id]);
                            }
                        }
                        //if (Settings::debug_msg) {
                        //    std::cout << "roll back" << std::endl;
                        //    std::cout << *(now_graph) << std::endl;
                        //}
                        id_set.erase(new_id);
                        id_map.erase(new_id);
                    }
                }
            }
        }
    } else {
        for (auto &other_iter : freq_edge_pairs[seed_node_id]) {
            int other_id = other_iter;
            std::cout << "other id: " << seed_node_id << std::endl;
            std::cout << "other id: " << other_id << std::endl;
            id_set = std::set<int>();
            id_map = std::map<int, int>();
            id_set.insert(seed_node_id);
            id_set.insert(other_id);
            id_map[seed_node_id] = 0;
            id_map[other_id] = 1;
            std::string el = graph->get_edge_label(seed_node_id, other_id);
            std::string nl0 = graph->get_node_with_id(seed_node_id)->get_label();
            std::string nl1 = graph->get_node_with_id(other_id)->get_label();
            now_graph = new GraphX(1, graph_type);
//            GraphX *pre_graph = new GraphX(now_graph);
            now_graph->add_node(0, nl0);
            now_graph->add_node(1, nl1);
            now_graph->add_edge(0, 1, el);
            //if (Settings::debug_msg) {
            //    std::cout << "test now_graph" << std::endl;
            //    std::cout << *(now_graph) << std::endl;
            //}
            for (auto &old_id : id_set) {
                for (auto &new_id: freq_edge_pairs[old_id]) {
                    if (id_set.find(new_id) != id_set.end()) {
                        continue;
                    }
                    DFS(old_id, new_id);
                    for (auto &pid : id_set) {
                        if (freq_edge_pairs[pid].find(new_id) != freq_edge_pairs[pid].end()) {
                            now_graph->remove_edge(id_map[pid], id_map[new_id]);
                        }
                    }
                    //if (Settings::debug_msg) {
                    //    std::cout << "roll back" << std::endl;
                    //    std::cout << *(now_graph) << std::endl;
                    //}
                    id_set.erase(new_id);
                    id_map.erase(new_id);
                }
            }
        }
    }
    for (auto &frequent_pattern : frequent_pattern_vec) {
        std::cout << *(frequent_pattern->get_graph()) << std::endl;
    }
    print_frequent_module();
}

void MyMiner::DFS(int old_id, int new_id) {
    id_set.insert(new_id);
    id_map[new_id] = int(id_set.size()-1);
    std::string new_nl = graph->get_node_with_id(new_id)->get_label();
    std::string new_el = graph->get_edge_label(old_id, new_id);
    now_graph->add_node(int(id_set.size()-1), new_nl);
    now_graph->add_edge(id_map[old_id], id_map[new_id], new_el);
    for (auto &pre_id : id_set) {
        if (pre_id != old_id && freq_edge_pairs[pre_id].find(new_id) != freq_edge_pairs[pre_id].end()) {
            std::string other_el = graph->get_edge_label(pre_id, new_id);
            now_graph->add_edge(id_map[pre_id], id_map[new_id], other_el);
        }
    }
    domains_solutions.clear();
    std::string sig = now_graph->get_canonical_label();
    if (cl_set.find(sig) != cl_set.end()) {
        return;
    }
    //if (Settings::debug_msg) {
    //    std::cout << "DFS graph" << std::endl;
    //    std::cout << *(now_graph) << std::endl;
    //}
    auto *new_candidate = new Pattern(now_graph, true);
    int freq = GraMiCounter::is_frequent(graph, new_candidate, support, -1, domains_solutions);
    //std::cout << *(now_graph) << std::endl;
    if (freq < support) {
        //for (auto &pre_id : id_set) {
        //    std::cout << "test2" << std::endl;
        //    if (freq_edge_pairs[pre_id].find(new_id) != freq_edge_pairs[pre_id].end()) {
        //        std::cout << "test3" << std::endl;
        //        std::cout << pre_id << std::endl;
        //        std::cout << id_map[pre_id] << std::endl;
        //        std::cout << new_id << std::endl;
        //        std::cout << id_map[new_id] << std::endl;
        //        now_graph->remove_edge(id_map[pre_id], id_map[new_id]);
        //        std::cout << "test4" << std::endl;
        //    }
        //    std::cout << "test5" << std::endl;
        //}
        //std::cout << "test6" << std::endl;
//        now_graph->remove_node_ignore_edges();
        //id_set.erase(new_id);
        //id_map.erase(new_id);
        return;
    }
    else {
        std::cout << "frequent!" << std::endl;
        std::cout << *(new_candidate->get_graph()) << std::endl;
        for (auto &domains_solution : domains_solutions) {
            std::cout << "domain ID: " << domains_solution.first << std::endl;
            for (auto &v : domains_solution.second) {
                std::cout << "value ID: " << v << std::endl;
            }
        }
        std::cout << "------------------------------------------" << std::endl;
        //std::cout << "frequent!" << std::endl;
        if ((frequent_patterns.size() - 1) < new_candidate->get_size()) {
            frequent_patterns.insert(frequent_patterns.begin() + new_candidate->get_size(), new CLMap());
        }
        cl_set.insert(sig);
        frequent_patterns[new_candidate->get_size()]->add_pattern(new_candidate);
        frequent_pattern_vec.emplace_back(new_candidate);
        frequent_patterns_domain[int(frequent_pattern_vec.size() - 1)] = domains_solutions;

        for (auto &pre_id : id_set) {
            for (auto &next_id: freq_edge_pairs[pre_id]) {
                if (id_set.find(next_id) != id_set.end()) {
                    continue;
                }
                DFS(pre_id, next_id);
                for (auto &pid : id_set) {
                    if (freq_edge_pairs[pid].find(next_id) != freq_edge_pairs[pid].end()) {
                        now_graph->remove_edge(id_map[pid], id_map[next_id]);
                    }
                }
                id_set.erase(next_id);
                id_map.erase(next_id);
            }
        }
    }
}

char *MyMiner::pop_my_candidate(std::vector<CLMap *> &candidates, std::map<int, Pattern *> &currently_checking, int support,
                                double approximate) {
    if (Settings::debug_msg)
        std::cout << "try to pop a candidate ..." << std::endl;
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
            std::cout << "No more candidate ..." << std::endl;
        result[0] = '0';
        return result;
    }

    if (Settings::debug_msg)
        std::cout << "Popping a candidate ..." << std::endl;
    //get a candidate, if there is
    if (current_candidates->get_size() > 0) {
        CLMap_Iterator iter = current_candidates->get_first_element();
        std::string key = iter.key;
        Pattern *candidate = iter.pattern;
        current_candidates->remove_pattern(candidate);

        // sendACandidateApprox(key, candidate, currently_checking, destination);
        if (currently_checking.find(candidate->get_id()) != currently_checking.end()) {
            std::cout << "candidate->get_id() = " << candidate->get_id() << std::endl << std::flush;
            exit(0);
        }

        if (Settings::debug_msg) {
            std::cout << "candidate ID: " << candidate->get_id() << std::endl;
        }
        currently_checking[candidate->get_id()] = candidate;
//		currently_checking.insert(std::pair<int, Pattern*>(candidate->get_id(), candidate));

        Miner::num_is_freq_calls++;

        std::ostringstream temp_os;
        temp_os << *(candidate->get_graph());
        char graph_str[2000];
        sprintf(graph_str, "%d,%s\t", candidate->get_id(), temp_os.str().c_str());
        if (Settings::debug_msg)
            std::cout << "now pattern: " << graph_str << std::endl;

        is_freq = work_count(graph_str, support, approximate);
        int fre = is_freq ? 1 : 0;
        sprintf(result, "%d,%d", fre, candidate->get_id());
    }

    return result;
}

void clear_all_mp(std::tr1::unordered_map<std::string, void *> &all_mp) {
    for (std::tr1::unordered_map<std::string, void *>::iterator iter = all_mp.begin(); iter != all_mp.end(); iter++) {
//        delete iter->second;
        delete (Pattern*) iter->second;
    }
    all_mp.clear();
}

bool MyMiner::work_count(char *graph_str, int support, double approximate) {
    std::tr1::unordered_map<std::string, void *> all_mps;
    GraphX *subgraph = new GraphX(1, false);

    int candidate_id;
    char subgraph_temp[500];
    sscanf(graph_str, "%d,%[^\t]", &candidate_id, subgraph_temp);
    std::stringstream ss;
    ss << subgraph_temp;
    std::string subgraph_str = ss.str();
    subgraph->load_from_string(subgraph_str, all_mps);
    clear_all_mp(all_mps);
    Pattern *new_candidate = new Pattern(subgraph, false);

    unsigned long start_time = get_ms_of_day();
    int freq = get_freq(new_candidate, support, approximate);
    bool is_freq;
    if (freq >= support) {
        is_freq = true;
//        std::cout << "test in fuc work_count" << std::endl;
//        std::cout << *subgraph;
//        for (auto & domains_solution : domains_solutions) {
//            std::cout << "domain ID: " << domains_solution.first << "domain size: " << domains_solution.second.size() << std::endl;
//            for (auto & value_iter : domains_solution.second) {
//                std::cout << "value ID: " << value_iter;
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
        std::cout << "work_count True!" << std::endl;
    }
    return is_freq;
}


void MyMiner::print_total_expected_time() {
    std::vector<CLMap *> *all_patterns[2];
    all_patterns[0] = &frequent_patterns;
    all_patterns[1] = &infrequent_patterns;

    unsigned long int totat_time = 0;

    //add to candidates
    for (int l = 0; l < 2; l++) {
        for (std::vector<CLMap *>::iterator iter = all_patterns[l]->begin(); iter != all_patterns[l]->end(); iter++) {
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
    //std::cout << "now counting freq!" << std::endl;
    std::vector<unsigned long> list_of_num_of_iters;
    int freq = 0;
    freq = GraMiCounter::is_frequent(graph, candidate, support, approximate, domains_solutions);
    if (Settings::debug_msg) {
        if (freq >= support) {
            std::cout << *candidate->get_graph();
            for (auto &domains_solution: domains_solutions) {
                std::cout << "domain ID: " << domains_solution.first << "domain size: " << domains_solution.second.size()
                     << std::endl;
                for (auto &value_iter: domains_solution.second) {
                    std::cout << "value ID: " << value_iter;
                }
            }
        }
    }
    return freq;
}


void MyMiner::print_result(std::tr1::unordered_set<int> delete_pattern_id) {
    std::cout << "print final result!";
    int num = 0;
    int cnt = 0;
    for (auto &pattern: frequent_pattern_vec) {
        if (delete_pattern_id.find(num) != delete_pattern_id.end()) {
            num++;
            continue;
        }
        std::cout << std::endl;
        std::cout << "*****frequent pattern " << cnt << "*****" << std::endl;
        std::cout << *(frequent_pattern_vec[num]->get_graph()) << std::endl;
        auto now_domain = frequent_patterns_domain[num];
        for (auto &map_iter: now_domain) {
            std::cout << "domain: " << map_iter.first << std::endl;
            std::cout << "values: " << std::endl;
            std::set<int> st = map_iter.second;
            for (auto &value_iter: st) {
                std::cout << value_iter << std::endl;
            }
        }
        std::cout << "**************************" << std::endl;
        num++;
        cnt++;
    }
}

void MyMiner::print_frequent_module() {
    std::cout << "print final result!";
    int cnt = 0;
    for (auto &pattern: frequent_pattern_vec) {
        std::cout << std::endl;
        std::cout << "*****frequent pattern: " << cnt << "*****" << std::endl;
        std::cout << *(frequent_pattern_vec[cnt]->get_graph()) << std::endl;
        auto now_domain = frequent_patterns_domain[cnt];
        for (auto &map_iter: now_domain) {
            std::cout << "domain: " << map_iter.first << std::endl;
            std::cout << "values: " << std::endl;
            std::set<int> st = map_iter.second;
            for (auto &value_iter: st) {
                std::cout << value_iter << std::endl;
            }
        }
        std::cout << "**************************" << std::endl;
        cnt++;
    }
}

void MyMiner::print_domains() {
    for (auto &pattern_iter: frequent_patterns_domain) {
        std::cout << "subgraph" << pattern_iter.first << ":" << std::endl;
        std::cout << *(frequent_pattern_vec[pattern_iter.first]->get_graph()) << std::endl;
        for (auto &map_iter: pattern_iter.second) {
            std::cout << "domain: " << map_iter.first << std::endl;
            std::cout << "values: " << std::endl;
            std::set<int> st = map_iter.second;
            for (auto &value_iter: st) {
                std::cout << value_iter << std::endl;
            }
        }
    }
}

void MyMiner::write_solutions(const std::string &path) {
    std::string graph_file = path + "/graph.data";
    std::ofstream out(graph_file);
    for (auto &pattern_iter: frequent_patterns_domain) {
        out << "subgraph" << pattern_iter.first << ":" << std::endl;
        out << *(frequent_pattern_vec[pattern_iter.first]->get_graph()) << std::endl;
        for (auto &map_iter: pattern_iter.second) {
            out << "domain: " << map_iter.first << std::endl;
            out << "solutions: " << std::endl;
            std::set<int> st = map_iter.second;
            for (auto &value_iter: st) {
                out << value_iter << std::endl;
            }
        }
    }
}

