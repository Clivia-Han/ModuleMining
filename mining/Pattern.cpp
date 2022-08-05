/**
 * represents a frequent subgraph
 */

#include "Pattern.h"
#include "Settings.h"
#include "core_file.h"
#include "mining_utils.h"


int Pattern::max_pattern_id = 0;

bool PrimaryGraph::same_with(PrimaryGraph *other_pg) {
    if (this->graph->get_nodes_num() != other_pg->graph->get_nodes_num() ||
        this->graph->get_edges_num() != other_pg->graph->get_edges_num())
        return false;

    std::vector<std::map<int, int> *> result;
    std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> domains_values;

    this->get_graph()->is_isomorphic(other_pg->get_graph(), result, domains_values);

    bool b;

    if (!result.empty())
        b = true;
    else
        b = false;

    for (auto & iter1 : result) {
        delete iter1;
    }
    result.clear();

    return b;
}

bool PrimaryGraph::same_with(MyGraph *other_pg) {
    if (this->graph->get_nodes_num() != other_pg->get_nodes_num() ||
        this->graph->get_edges_num() != other_pg->get_edges_num())
        return false;

    std::vector<std::map<int, int> *> result;
    std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> domains_values;

    this->get_graph()->is_isomorphic(other_pg, result, domains_values);

    bool b;

    if (!result.empty())
        b = true;
    else
        b = false;

    for (auto & iter1 : result) {
        delete iter1;
    }
    result.clear();

    return b;
}

Pattern::Pattern(MyGraph *graph, bool copyGraph) {
    this->graph_copied = copyGraph;
    this->result_exact = false;
    this->ID = Pattern::max_pattern_id;
    Pattern::max_pattern_id++;
    this->frequency = -1;
    if (copyGraph)
        this->graph = new MyGraph(graph);
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
    this->graph = new MyGraph(pattern->get_graph());
    this->graph_copied = true;
    this->predicted_time = pattern->get_predicted_time();
    this->set_invalid_col(pattern->get_invalid_col(), pattern->get_predicted_valids());

    this->result_exact = pattern->is_result_exact();
    if (result_exact)
        this->frequency = pattern->get_frequency();

    init();

    this->combine(pattern);
}

int Pattern::get_id() {
    return ID;
}

void Pattern::init() {
    int num_nodes = graph->get_nodes_num();
    for (int i = 0; i < num_nodes; i++)
        occurences.push_back(new std::tr1::unordered_set<int>());
    predicted_time = 0;

    temp_mni_table = 0;

//	if(Settings::postpone_expensive_nodes)
//		postpone_expensive_nodes = true;
//	else
//		postpone_expensive_nodes = false;

    temp_mni_table = new int[this->graph->get_nodes_num()];
    postponedNodes_mniTable = new int[this->graph->get_nodes_num()];
}

void Pattern::set_invalid_col(int inv_c, int p_valids) {
    invalid_col = inv_c;
    predicted_valids = p_valids;
    if (inv_c == -1)
        postpone_expensive_nodes = false;
}

/**
 * get the joining primary graphs of the two patterns
 */
std::set<std::pair<PrimaryGraph *, PrimaryGraph *> > Pattern::get_joining_pg(Pattern *pattern) {
    std::set<std::pair<PrimaryGraph *, PrimaryGraph *> > l_list;

    for (auto pg1 : primary_graphs) {
        for (auto pg2 : pattern->primary_graphs) {
            if (pg1->same_with(pg2)) {
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

    if (graph->get_nodes_num() == 0)
        return -1;

    int min = occurences[0]->size();
    for (int i = 1; i < graph->get_nodes_num(); i++) {
        if (min > occurences[i]->size())
            min = occurences[i]->size();
    }

    return min;
}

std::map<int, std::set<int>> Pattern::get_domain_values() {
    std::map<int, std::set<int>> domain_nodes;
    for (int i = 0; i < graph->get_nodes_num(); i++) {
        std::set<int> st(occurences[i]->begin(), occurences[i]->end());
        domain_nodes.insert(std::pair<int, std::set<int>>(i, st));
    }
    return domain_nodes;
}

void Pattern::add_node(int node_id, int pattern_node_id) {
    occurences[pattern_node_id]->insert(node_id);
}

std::string Pattern::to_string() {
    std::string str = int_to_string(get_frequency()) + "\n";
    for (int i = 0; i < graph->get_nodes_num(); i++) {
        str = str + int_to_string(i) + " with label: " + graph->get_node_with_id(i)->get_label() +
              "\nNodes list:\n";

        for (int iter : *occurences[i]) {
            str = str + "," + int_to_string(iter);
        }
        str = str + "\n";
    }
    return str;
}

void Pattern::combine(Pattern *other_p, int add_to_id) {
    invalidate_frequency();
    for (int i = 0; i < graph->get_nodes_num(); i++) {
        for (std::tr1::unordered_set<int>::iterator iter = other_p->get_occurences()->at(i)->begin(); iter !=
                                                                                                 other_p->get_occurences()->at(
                                                                                                         i)->end(); ++iter) {
            occurences[i]->insert((*iter) + add_to_id);
        }
    }
}

int Pattern::get_size() {
    return graph->get_nodes_num();
}

bool Pattern::has_unique_labels() {
    std::tr1::unordered_set<std::string> labels;
    for (std::tr1::unordered_map<int, MyNode *>::const_iterator iter = graph->get_nodes_iterator(); iter !=
                                                                                                    graph->get_nodes_end_iterator(); iter++) {
        std::string label = iter->second->get_label();
        if (labels.find(label) == labels.end())
            labels.insert(label);
        else
            return false;
    }
    return true;
}

void vect_map_destruct(std::vector<std::map<std::string, Pattern *> *> vm) {
    for (auto & iter1 : vm) {
        for (auto & iter2 : *iter1) {
            delete (iter2.second);

        }

        iter1->clear();
        delete iter1;

    }
}

Pattern::~Pattern() {
    if (this->graph_copied)
        delete graph;
    for (auto & occurence : occurences)
        delete occurence;

    for (auto & primary_graph : primary_graphs) {
        delete primary_graph;
    }

    primary_graphs.clear();
    if (temp_mni_table != 0)
        delete[] temp_mni_table;

    if (postponedNodes_mniTable != 0)
        delete[] postponedNodes_mniTable;
}
