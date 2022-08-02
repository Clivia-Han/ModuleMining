/**
*
Miner.cpp
 */

#include "Miner.h"
#include "Settings.h"
#include "core_file.h"
#include "CSPSolver.h"
#include "mining_utils.h"

void Miner::generate_now_graph(System &sym, int support, int seed_node_id) {
    graph = new MyGraph(1);
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
                        // If the number of times a node with a label appears in the graph is less than the threshold,
                        // it means that all edges connected to a node with this label are infrequent
                        std::tr1::unordered_map<std::string, std::set<int> *>::iterator iter2 = nodes_by_label.find(
                                iter->first);
                        if (iter2 == nodes_by_label.end()) {
                            std::cout << "Error: isufnm44" << std::endl;
                            exit(0);
                        }
                        // traverse all nodes with this label, iter2 is the node among them,
                        // ignore this node and the edge connected to this node
                        std::set<int> *nodes_to_remove = iter2->second;
                        for (auto iter1 = nodes_to_remove->begin();
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
    // traverse all edges and add frequent edges to freqEdges
    for (auto & iter : edge_to_freq) {
        if (((Pattern *) (iter.second))->get_frequency() >= support) {
            Pattern *p = (Pattern *) (iter.second);
            std::cout << *(p->get_graph()) << std::endl;

            std::string key = iter.first;
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

            std::string now_sig = p->get_graph()->get_sig();

//            freq_pattern_pairs[key] = freq_edge_pairs;
            if (sig_set.count(now_sig) < 1) {
                if (seed_node_id != -1 && !flag)
                    continue;
                sig_set.insert(now_sig);
                frequent_edges.add_pattern(p);
                frequent_pattern_vec.push_back(p);
                frequent_patterns_domain.insert(std::pair<int, std::map<int, std::set<int>>>(frequent_pattern_vec.size() - 1,
                                                                                   p->get_domain_values()));
            }
        } else
            delete (Pattern *) (iter.second);
    }
    edge_pairs.clear();
    std::cout << "Master: loop finishes" << std::endl;

    std::cout << " data loaded successfully!" << std::endl;
}

/**
 * start the mining process, given the filename (file base name for the partitions)
 * , and the required support threshold
 */
void Miner::start_mining_module(System &sym, int support, int seed_node_id, const std::string& path) {
    this->support = support;

    //load the graph
    long long start = get_msec();
    generate_now_graph(sym, support, seed_node_id);

    if (frequent_edges.get_size() == 0) {
        std::cout << "No frequent patterns found! Exiting" << std::endl;
        exit(0);
    }

    long long end = get_msec();
    long long elapsed = end - start;

    std::cout << "Loading took " << (elapsed / 1000) << " sec and " << (elapsed % 1000) << " ms" << std::endl;


//    Settings::graphLoadingTime = elapsed;

    SigMap *sig_map = new SigMap();

    sig_map->add_all(&frequent_edges);

    SigMap *empty_map = new SigMap();
    frequent_patterns.insert(frequent_patterns.begin() + 1, empty_map);
    frequent_patterns.insert(frequent_patterns.begin() + 2, sig_map);//frequent edges!

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
                now_graph = new MyGraph(1);
//            MyGraph *pre_graph = new MyGraph(now_graph);
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
//            std::cout << "other id: " << seed_node_id << std::endl;
//            std::cout << "other id: " << other_id << std::endl;
            id_set = std::set<int>();
            id_map = std::map<int, int>();
            id_set.insert(seed_node_id);
            id_set.insert(other_id);
            id_map[seed_node_id] = 0;
            id_map[other_id] = 1;
            std::string el = graph->get_edge_label(seed_node_id, other_id);
            std::string nl0 = graph->get_node_with_id(seed_node_id)->get_label();
            std::string nl1 = graph->get_node_with_id(other_id)->get_label();
            now_graph = new MyGraph(1);
//            MyGraph *pre_graph = new MyGraph(now_graph);
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

void Miner::DFS(int old_id, int new_id) {
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
    std::string sig = now_graph->get_sig();
    if (sig_set.find(sig) != sig_set.end()) {
        return;
    }
    //if (Settings::debug_msg) {
    //    std::cout << "DFS graph" << std::endl;
    //    std::cout << *(now_graph) << std::endl;
    //}
    auto *new_candidate = new Pattern(now_graph, true);
    int freq = CSPSolver::get_frequency(graph, new_candidate, support, -1, domains_solutions);
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
        if(Settings::debug_msg) {
            std::cout << "frequent!" << std::endl;
            std::cout << *(new_candidate->get_graph()) << std::endl;
            for (auto &domains_solution: domains_solutions) {
                std::cout << "domain ID: " << domains_solution.first << std::endl;
                for (auto &v: domains_solution.second) {
                    std::cout << "value ID: " << v << std::endl;
                }
            }
            std::cout << "------------------------------------------" << std::endl;
        }
        //std::cout << "frequent!" << std::endl;
        if ((frequent_patterns.size() - 1) < new_candidate->get_size()) {
            frequent_patterns.insert(frequent_patterns.begin() + new_candidate->get_size(), new SigMap());
        }
        sig_set.insert(sig);
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

void Miner::print_frequent_module() {
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

void Miner::print_domains() {
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

void Miner::write_solutions(const std::string &path) {
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

Miner::Miner() {
    candidates.insert(candidates.begin(), new SigMap());
    candidates.insert(candidates.begin() + 1,
                      new SigMap());

    frequent_patterns.insert(frequent_patterns.begin(), new SigMap());

    infrequent_patterns.insert(infrequent_patterns.begin(), new SigMap());
    infrequent_patterns.insert(infrequent_patterns.begin() + 1, new SigMap());
}

Miner::~Miner() {
    if (graph != 0)
        delete graph;

    vect_map_destruct(frequent_patterns);
    vect_map_destruct(infrequent_patterns);
    vect_map_destruct(candidates);
}

