#include <iostream>
#include <sstream>
#include<sys/time.h>
#include <stdlib.h>
#include <string.h>
#include "Miner.h"
#include "utils.h"

long long getmsofday_2() {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return (long long) tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int Miner::num_is_freq_calls = 0;
int Miner::num_freqs = 0;
int Miner::num_infreq = 0;

Miner::Miner() {
    candidates.insert(candidates.begin(), new CLMap());//graphs with zero edges!
    candidates.insert(candidates.begin() + 1,
                      new CLMap());//graphs with one edge! no ned also, because we have special treatment for graphs with one edge

    frequent_patterns.insert(frequent_patterns.begin(), new CLMap());//frequent patterns with zero edges!

    infrequent_patterns.insert(infrequent_patterns.begin(), new CLMap());//infrequents with zero edges!
    infrequent_patterns.insert(infrequent_patterns.begin() + 1, new CLMap());//infrequents with one edge!
}

void Miner::load_graph(string base_name, int graph_type, int support, CLMap &freq_edges) {
    this->support = support;

    tr1::unordered_map<string, void *> all_mps;

    int nodes_counter = 0;

    //load graph data
    std::stringstream ss;
    ss << base_name;
    string part_file_name = ss.str();
    graph = new GraphX(1, graph_type);//create a graph with id=its partition id
    graph->set_frequency(support);
    cout << "Master: created a graph object" << endl;
    if (!graph->load_from_file(part_file_name, all_mps)) {
        cout << "Master: delete a graph object" << endl;
        delete graph;
    }
    nodes_counter += graph->get_num_of_nodes();

    cout << "Master: loop starts" << endl;
    //遍历所有边，将频繁边加入freqEdges
    for (tr1::unordered_map<string, void *>::iterator iter = all_mps.begin(); iter != all_mps.end(); ++iter) {
        if (((Pattern *) ((*iter).second))->get_frequency() >= support) {
            Pattern *p = (Pattern *) ((*iter).second);
            cout << *(p->get_graph()) << endl;
            freq_edges.add_pattern(p);
            frequent_pattern_vec.push_back(p);
            frequent_patterns_domain.insert(std::pair<int, map<int, set<int>>>(frequent_pattern_vec.size() - 1,
                                                                               p->get_domain_values()));
        } else
            delete (Pattern *) ((*iter).second);
    }
    cout << "Master: loop finishes" << endl;

    cout << " data loaded successfully!" << endl;
}

void Miner::load_graph_with_pairs(string base_name, int graph_type, int support, CLMap &freq_edges) {
    this->support = support;

    tr1::unordered_map<string, void *> all_mps;
    map<string, map<int, set<int>>> edge_pairs;

    int nodes_counter = 0;

    //load graph data
    std::stringstream ss;
    ss << base_name;
    string part_file_name = ss.str();
    graph = new GraphX(1, graph_type);//create a graph with id=its partition id
    graph->set_frequency(support);
    cout << "Master: created a graph object" << endl;
    if (!graph->load_from_file(part_file_name, all_mps, edge_pairs)) {
        cout << "Master: delete a graph object" << endl;
        delete graph;
    }
    nodes_counter += graph->get_num_of_nodes();

    cout << "Master: loop starts" << endl;
    //遍历所有边，将频繁边加入freqEdges
    for (tr1::unordered_map<string, void *>::iterator iter = all_mps.begin(); iter != all_mps.end(); ++iter) {
        if (((Pattern *) ((*iter).second))->get_frequency() >= support) {
            Pattern *p = (Pattern *) ((*iter).second);
            cout << *(p->get_graph()) << endl;
            freq_edges.add_pattern(p);

            string key = iter->first;

            for (auto &edge_pair : edge_pairs[key]) {
                if (freq_edge_pairs.find(edge_pair.first) == freq_edge_pairs.end()) {
                    freq_edge_pairs[edge_pair.first] = set<int>();
                }
                for (auto &x : edge_pair.second) {
                    freq_edge_pairs[edge_pair.first].insert(x);
                }
            }
//            freq_pattern_pairs[key] = freq_edge_pairs;

            frequent_pattern_vec.push_back(p);
            frequent_patterns_domain.insert(std::pair<int, map<int, set<int>>>(frequent_pattern_vec.size() - 1,
                                                                               p->get_domain_values()));
        } else
            delete (Pattern *) ((*iter).second);
    }
    edge_pairs.clear();
    cout << "Master: loop finishes" << endl;

    cout << " data loaded successfully!" << endl;
}

void Miner::extend_freq_edges() {
    CLMap *two_edges_candidate = new CLMap();
    candidates.insert(candidates.begin() + 2, two_edges_candidate);

    long long total_elapsed = 0;

    CLMap_Iterator iter1 = frequent_edges.get_first_element();
    while (iter1.pattern != 0)
        //for(map<string, Pattern*>::iterator iter1 = frequent_edges.begin();iter1!=frequent_edges.end();++iter1)
    {
        Pattern *edge1 = iter1.pattern;//->second;

        double l1_0 = edge1->get_graph()->get_node_with_id(0)->get_label();
        double l1_1 = edge1->get_graph()->get_node_with_id(1)->get_label();

        CLMap_Iterator iter2 = iter1.get_copy();
        while (iter2.pattern != 0)
            //for(map<string, Pattern*>::iterator iter2 = iter1;iter2!=frequent_edges.end();++iter2)
        {
            Pattern *edge2 = iter2.pattern;//->second;
            frequent_edges.advance_iterator(iter2);

            double l2_0 = edge2->get_graph()->get_node_with_id(0)->get_label();
            double l2_1 = edge2->get_graph()->get_node_with_id(1)->get_label();
            double edge2_label = edge2->get_graph()->get_edge_label(0, 1);

            //connectType refers to the way they can connect, -q means they can not connect, 0 means 0 and 0, 1 means 0 and 1, 2 means 1 and 0, and 3 means 1 and 1
            int last_connect_type = -1;
            while (true) {
                int connect_type = -1;

                //check how can they (edge1 and edge2) connect.
                if (l1_0 == l2_0 && last_connect_type < 0)
                    connect_type = 0;
                else if (l1_0 == l2_1 && last_connect_type < 1)
                    connect_type = 1;
                else if (l1_1 == l2_0 && last_connect_type < 2)
                    connect_type = 2;
                else if (l1_1 == l2_1 && last_connect_type < 3)
                    connect_type = 3;

                //could not find an extension by edge1 and edge2
                if (connect_type == -1)
                    break;

                last_connect_type = connect_type;

                long long start = getmsofday_2();

                Pattern *candidate = new Pattern(edge1);
                candidate->invalidate_frequency();

                long long end = getmsofday_2();
                total_elapsed += (end - start);

                switch (connect_type) {
                    case 0://00
                        candidate->extend(0, 2, l2_1, edge2_label);
                        break;
                    case 1://01
                        candidate->extend(0, 2, l2_0, edge2_label);
                        break;
                    case 2://10
                        candidate->extend(1, 2, l2_1, edge2_label);
                        break;
                    case 3://11
                        candidate->extend(1, 2, l2_0, edge2_label);
                        break;
                }

                if (two_edges_candidate->get_pattern(candidate) != 0)
                    delete candidate;
                else {
                    candidate->generate_primary_graphs();
                    cout << *(candidate->get_graph()) << endl;
                    bool b = two_edges_candidate->add_pattern(candidate);
                    if (!b) delete candidate;
                }
            }

        }

        frequent_edges.advance_iterator(iter1);
    }
}

void Miner::set_frequent_edges(CLMap &freqEdges) {
    this->frequent_edges.add_all(&freqEdges);
}

void Miner::print_candidates() {
    print(candidates);
}

void Miner::print_result() {
    print(frequent_patterns);
}

void Miner::remove_pattern(Pattern *pattern, vector<CLMap *> &data) {
    CLMap *tempList = data.at(pattern->get_size());
    tempList->remove_pattern(pattern);
}

void Miner::print(vector<CLMap *> &data) {
    //count the total number of frequent patterns
    int size = 0;
    for (unsigned int i = 0; i < data.size(); i++) {
        size += data[i]->get_size();
    }
    int count = 1;
    cout << "[Miner] There are " << size << " frequent patterns, and they are:" << endl;
    for (unsigned int i = 0; i < data.size(); i++) {
        cout << "With " << (i) << " edges:" << endl;
        data[i]->print();
    }
}

Miner::~Miner() {
    if (graph != 0)
        delete graph;

    vect_map_destruct(frequent_patterns);
    vect_map_destruct(infrequent_patterns);
    vect_map_destruct(candidates);
}
