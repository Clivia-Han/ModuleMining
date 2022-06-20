/**
 * represents a frequent subgraph
 */

#include "Pattern.h"
#include "Settings.h"
#include "core_file.h"
#include "mining_utils.h"


int Pattern::max_pattern_id = 0;

bool PrimaryGraph::is_the_same_with(PrimaryGraph *other_pg) {
    if (this->graph->get_num_of_nodes() != other_pg->graph->get_num_of_nodes() ||
        this->graph->get_num_of_edges() != other_pg->graph->get_num_of_edges())
        return false;

    std::vector<std::map<int, int> *> result;
    std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> domains_values;

    this->get_graph()->is_isomorphic(other_pg->get_graph(), result, domains_values);

    bool b;

    if (result.size() > 0)
        b = true;
    else
        b = false;

    for (std::vector<std::map<int, int> *>::iterator iter1 = result.begin(); iter1 != result.end(); iter1++) {
        delete (*iter1);
    }
    result.clear();

    return b;
}

bool PrimaryGraph::is_the_same_with(GraphX *other_pg) {
    if (this->graph->get_num_of_nodes() != other_pg->get_num_of_nodes() ||
        this->graph->get_num_of_edges() != other_pg->get_num_of_edges())
        return false;

    std::vector<std::map<int, int> *> result;
    std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> domains_values;

    this->get_graph()->is_isomorphic(other_pg, result, domains_values);

    bool b;

    if (result.size() > 0)
        b = true;
    else
        b = false;

    for (std::vector<std::map<int, int> *>::iterator iter1 = result.begin(); iter1 != result.end(); iter1++) {
        delete (*iter1);
    }
    result.clear();

    return b;
}

Pattern::Pattern(GraphX *graph, bool copyGraph) {
    this->graph_copied = copyGraph;
    this->result_exact = false;
    this->ID = Pattern::max_pattern_id;
    Pattern::max_pattern_id++;
    this->frequency = -1;
    if (copyGraph)
        this->graph = new GraphX(graph);
    else
        this->graph = graph;

    this->max_iters = Settings::postpone_nodes_after_iterations;

    init();
}

Pattern::Pattern(Pattern *pattern) {
    this->ID = Pattern::max_pattern_id;
    Pattern::max_pattern_id++;
    this->frequency = pattern->get_frequency();
    //get a similar graph
    this->graph = new GraphX(pattern->get_graph());
    this->graph_copied = true;
    this->predicted_time = pattern->get_predicted_time();
    this->set_invalid_col(pattern->get_invalid_col(), pattern->get_predicted_valids());

    this->result_exact = pattern->is_result_exact();
    if (result_exact)
        this->frequency = pattern->get_frequency();

    init();

    this->combine(pattern);
}

void Pattern::reset_mni() {
    if (subtasking_fixed > 0) {
        for (int i = 0; i < this->graph->get_num_of_nodes(); i++) {
            temp_mni_table[i] = 0;
            postponedNodes_mniTable[i] = 0;
        }
    }
}

int Pattern::get_id() {
    return ID;
}

void Pattern::init() {
    int num_nodes = graph->get_num_of_nodes();
    for (int i = 0; i < num_nodes; i++)
        occurences.push_back(new std::tr1::unordered_set<int>());
    predicted_time = 0;

    temp_mni_table = 0;

//	if(Settings::postpone_expensive_nodes)
//		postpone_expensive_nodes = true;
//	else
//		postpone_expensive_nodes = false;

    temp_mni_table = new int[this->graph->get_num_of_nodes()];
    postponedNodes_mniTable = new int[this->graph->get_num_of_nodes()];
}

void Pattern::set_invalid_col(int inv_c, int p_valids) {
    invalid_col = inv_c;
    predicted_valids = p_valids;
    if (inv_c == -1)
        postpone_expensive_nodes = false;
}

void Pattern::generate_primary_graphs() {
    if (this->primary_graphs.size() > 0) {
        if (Settings::debug_msg) {
            std::cout << "Already generated, return!" << std::endl;
        }
        return;
    }

    long long start1 = get_ms_of_day();

    //the set of already removed edges signatures, node1ID+"_"+node2ID if node1ID>node2ID, other wise it is node2ID+"_"+node1ID
    std::set<std::string> already_removed;
    //go over all nodes, try to remove an edge one at at time
    for (std::tr1::unordered_map<int, NodeX *>::const_iterator iter = graph->get_nodes_iterator(); iter !=
                                                                                              graph->get_nodes_end_iterator(); ++iter) {
        NodeX *node1 = iter->second;
        //if this node has one edge, then the other node (in this edge) should be treated as the source node
        if (node1->get_edges_size() == 1)
            continue;
        for (std::tr1::unordered_map<int, void *>::iterator iter = node1->get_edges_iterator(); iter !=
                                                                                           node1->get_edges_end_iterator(); ++iter) {
            EdgeX *edge = ((EdgeX *) iter->second);
            NodeX *node2 = edge->get_other_node();

            //check whether this edge has been removed before
            std::string node1_id = int_to_string(node1->get_id());
            std::string node2_id = int_to_string(node2->get_id());
            std::string sig;
            if (node1->get_id() > node2->get_id())
                sig = node1_id + "_" + node2_id;
            else
                sig = node2_id + "_" + node1_id;
            if (already_removed.find(sig) == already_removed.end()) {
                already_removed.insert(sig);
                GraphX *r_graph = new GraphX(graph);
                r_graph->remove_edge(node1->get_id(), node2->get_id());
                if (!r_graph->is_connected()) {
                    delete r_graph;
                    continue;
                }

                //check if the given primary graph is already generated before
                bool exists = false;
                for (std::list<PrimaryGraph *>::iterator iter = primary_graphs.begin();
                     iter != primary_graphs.end(); iter++) {
                    PrimaryGraph *pg_temp = (*iter);
                    if (pg_temp->is_the_same_with(r_graph)) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    PrimaryGraph *pg = new PrimaryGraph();
                    pg->set_values(r_graph, node1->get_id(), edge);
                    this->primary_graphs.push_back(pg);
                } else {
                    delete r_graph;
                }
            }
        }
    }
}

/**
 * get the joining primarygraphs of the two patterns
 */
std::set<std::pair<PrimaryGraph *, PrimaryGraph *> > Pattern::get_joining_pg(Pattern *pattern) {
    std::set<std::pair<PrimaryGraph *, PrimaryGraph *> > l_list;

    for (std::list<PrimaryGraph *>::iterator ii1 = primary_graphs.begin(); ii1 != primary_graphs.end(); ++ii1) {
        PrimaryGraph *pg1 = (*ii1);
        for (std::list<PrimaryGraph *>::iterator ii2 = pattern->primary_graphs.begin();
             ii2 != pattern->primary_graphs.end(); ++ii2) {
            PrimaryGraph *pg2 = (*ii2);
            if (pg1->is_the_same_with(pg2)) {
                l_list.insert(std::pair<PrimaryGraph *, PrimaryGraph *>(pg1, pg2));
            }
        }
    }

    return l_list;
}

void Pattern::set_frequency(int new_freq) {
    frequency = new_freq;
}

int Pattern::get_frequency() {
    if (frequency > -1)
        return frequency;

    if (graph->get_num_of_nodes() == 0)
        return -1;

    int min = occurences[0]->size();
    for (int i = 1; i < graph->get_num_of_nodes(); i++) {
        if (min > occurences[i]->size())
            min = occurences[i]->size();
    }

    return min;
}

std::map<int, std::set<int>> Pattern::get_domain_values() {
    std::map<int, std::set<int>> domain_nodes;
    for (int i = 0; i < graph->get_num_of_nodes(); i++) {
        std::set<int> st(occurences[i]->begin(), occurences[i]->end());
        domain_nodes.insert(std::pair<int, std::set<int>>(i, st));
    }
    return domain_nodes;
}

//void Pattern::borrow_time_infor(Pattern *other_pattern, int n_workers) {
//    //in case we already set subtaskingfixed, then there is no need to borrow information, because it was set earlier
//    if (this->subtasking_fixed > 1)
//        return;
//
//    if (Settings::debug_msg) {
//        std::cout << "Pattern#" << this->get_id() << " is borrowing time information from Pattern#"
//             << other_pattern->get_id() << "this.Subtasking = " << subtasking_fixed << ", other.subtasking = "
//             << other_pattern->get_subtasking_value_fixed() << std::endl;
//        std::cout << "Borrowed predicted time is: " << other_pattern->get_predicted_time() << std::endl;
//    }
//
//    this->predicted_time = other_pattern->get_predicted_time();
//    this->set_subtasking(other_pattern->get_subtasking_value_fixed(), n_workers);
//    this->set_invalid_col(other_pattern->get_invalid_col(), other_pattern->get_predicted_valids());
//
//    this->result_exact = other_pattern->is_result_exact();
//    if (result_exact)
//        this->frequency = other_pattern->get_frequency();
//
//    this->set_max_iters(other_pattern->get_max_iters());
//}

void Pattern::add_node(int node_id, int pattern_node_id) {
    occurences[pattern_node_id]->insert(node_id);
}

std::string Pattern::to_string() {
    std::string str = int_to_string(get_frequency()) + "\n";
    for (int i = 0; i < graph->get_num_of_nodes(); i++) {
        str = str + int_to_string(i) + " with label: " + graph->get_node_with_id(i)->get_label() +
              "\nNodes list:\n";

        for (std::tr1::unordered_set<int>::iterator iter = occurences[i]->begin(); iter != occurences[i]->end(); ++iter) {
            str = str + "," + int_to_string(*iter);
        }
        str = str + "\n";
    }
    return str;
}

void Pattern::combine(Pattern *other_p, int add_to_id) {
    invalidate_frequency();
    for (int i = 0; i < graph->get_num_of_nodes(); i++) {
        for (std::tr1::unordered_set<int>::iterator iter = other_p->get_occurences()->at(i)->begin(); iter !=
                                                                                                 other_p->get_occurences()->at(
                                                                                                         i)->end(); ++iter) {
            occurences[i]->insert((*iter) + add_to_id);
        }
    }
}

void Pattern::extend(int src_id, int dest_id, std::string dest_label, std::string edge_label) {

    if (graph->get_node_with_id(dest_id) == NULL)
        graph->add_node(dest_id, dest_label);
    graph->add_edge(src_id, dest_id, edge_label);

    int num_nodes = graph->get_num_of_nodes();
    while (occurences.size() < num_nodes)
        occurences.push_back(new std::tr1::unordered_set<int>());

    //delete primary graphs
    for (std::list<PrimaryGraph *>::iterator ii = primary_graphs.begin(); ii != primary_graphs.end(); ++ii) {
        delete (*ii);
    }

    primary_graphs.clear();
}

int Pattern::get_size() {
//    return graph->get_num_of_edges();
    return graph->get_num_of_nodes();
}

bool Pattern::has_unique_labels() {
    std::tr1::unordered_set<std::string> labels;
    for (std::tr1::unordered_map<int, NodeX *>::const_iterator iter = graph->get_nodes_iterator(); iter !=
                                                                                              graph->get_nodes_end_iterator(); iter++) {
        std::string label = iter->second->get_label();
        if (labels.find(label) == labels.end())
            labels.insert(label);
        else
            return false;
    }
    return true;
}

void Pattern::make_id_negative() {
    if (ID > 0)
        ID = ID * -1;
}

void vect_map_destruct(std::vector<std::map<std::string, Pattern *> *> vm) {
    for (std::vector<std::map<std::string, Pattern *> *>::iterator iter1 = vm.begin(); iter1 != vm.end(); ++iter1) {
        for (std::map<std::string, Pattern *>::iterator iter2 = (*iter1)->begin(); iter2 != (*iter1)->end(); iter2++) {
            delete (iter2->second);

        }

        (*iter1)->clear();
        delete (*iter1);

    }
}

Pattern::~Pattern() {
    if (this->graph_copied)
        delete graph;
    for (int i = 0; i < occurences.size(); i++)
        delete occurences.at(i);

    for (std::list<PrimaryGraph *>::iterator ii = primary_graphs.begin(); ii != primary_graphs.end(); ++ii) {
        delete (*ii);
    }

    primary_graphs.clear();
    if (temp_mni_table != 0)
        delete[] temp_mni_table;

    if (postponedNodes_mniTable != 0)
        delete[] postponedNodes_mniTable;
}
