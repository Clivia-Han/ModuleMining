#include "CLMap.h"
#include "Miner.h"
#include "core_file.h"
#include "mining_utils.h"


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

void Miner::load_graph(std::string base_name, int graph_type, int support, CLMap &freq_edges) {
    this->support = support;

    std::tr1::unordered_map<std::string, void *> all_mps;

    int nodes_counter = 0;

    //load graph data
    std::stringstream ss;
    ss << base_name;
    std::string part_file_name = ss.str();
    graph = new GraphX(1, graph_type);//create a graph with id=its partition id
    graph->set_frequency(support);
    std::cout << "Master: created a graph object" << std::endl;
    if (!graph->load_from_file(part_file_name, all_mps)) {
        std::cout << "Master: delete a graph object" << std::endl;
        delete graph;
    }
    nodes_counter += graph->get_num_of_nodes();

    std::cout << "Master: loop starts" << std::endl;
    //遍历所有边，将频繁边加入freqEdges
    for (std::tr1::unordered_map<std::string, void *>::iterator iter = all_mps.begin(); iter != all_mps.end(); ++iter) {
        if (((Pattern *) ((*iter).second))->get_frequency() >= support) {
            Pattern *p = (Pattern *) ((*iter).second);
            std::cout << *(p->get_graph()) << std::endl;
            freq_edges.add_pattern(p);
            frequent_pattern_vec.push_back(p);
            frequent_patterns_domain.insert(std::pair<int, std::map<int, std::set<int>>>(frequent_pattern_vec.size() - 1,
                                                                               p->get_domain_values()));
        } else
            delete (Pattern *) ((*iter).second);
    }
    std::cout << "Master: loop finishes" <<std::endl;

    std::cout << " data loaded successfully!" << std::endl;
}

void Miner::load_graph_with_pairs(std::string base_name, int graph_type, int support, CLMap &freq_edges, int seed_node_id) {
    this->support = support;

    std::tr1::unordered_map<std::string, void *> all_mps;
    std::map<std::string, std::map<int, std::set<int>>> edge_pairs;

    int nodes_counter = 0;

    //load graph data
    std::stringstream ss;
    ss << base_name;
    std::string part_file_name = ss.str();
    graph = new GraphX(1, graph_type);//create a graph with id=its partition id
    graph->set_frequency(support);
    std::cout << "Master: created a graph object" << std::endl;
    if (!graph->load_from_file(part_file_name, all_mps, edge_pairs)) {
        std::cout << "Master: delete a graph object" << std::endl;
        delete graph;
    }
    nodes_counter += graph->get_num_of_nodes();

    std::cout << "Master: loop starts" << std::endl;
    //遍历所有边，将频繁边加入frefqEdges
    for (std::tr1::unordered_map<std::string, void *>::iterator iter = all_mps.begin(); iter != all_mps.end(); ++iter) {
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
                freq_edges.add_pattern(p);
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

void Miner::extend_freq_edges() {
    CLMap *two_edges_candidate = new CLMap();
    candidates.insert(candidates.begin() + 2, two_edges_candidate);

    long long total_elapsed = 0;

    CLMap_Iterator iter1 = frequent_edges.get_first_element();
    while (iter1.pattern != 0)
        //for(std::map<std::string, Pattern*>::iterator iter1 = frequent_edges.begin();iter1!=frequent_edges.end();++iter1)
    {
        Pattern *edge1 = iter1.pattern;//->second;

        std::string l1_0 = edge1->get_graph()->get_node_with_id(0)->get_label();
        std::string l1_1 = edge1->get_graph()->get_node_with_id(1)->get_label();

        CLMap_Iterator iter2 = iter1.get_copy();
        while (iter2.pattern != 0)
            //for(std::map<std::string, Pattern*>::iterator iter2 = iter1;iter2!=frequent_edges.end();++iter2)
        {
            Pattern *edge2 = iter2.pattern;//->second;
            frequent_edges.advance_iterator(iter2);

            std::string l2_0 = edge2->get_graph()->get_node_with_id(0)->get_label();
            std::string l2_1 = edge2->get_graph()->get_node_with_id(1)->get_label();
            std::string edge2_label = edge2->get_graph()->get_edge_label(0, 1);

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
                    std::cout << *(candidate->get_graph()) << std::endl;
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

void Miner::remove_pattern(Pattern *pattern, std::vector<CLMap *> &data) {
    CLMap *tempList = data.at(pattern->get_size());
    tempList->remove_pattern(pattern);
}

void Miner::print(std::vector<CLMap *> &data) {
    //count the total number of frequent patterns
    int size = 0;
    for (unsigned int i = 0; i < data.size(); i++) {
        size += data[i]->get_size();
    }
    int count = 1;
    std::cout << "[Miner] There are " << size << " frequent patterns, and they are:" << std::endl;
    for (unsigned int i = 0; i < data.size(); i++) {
        std::cout << "With " << (i) << " edges:" << std::endl;
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
