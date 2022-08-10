#include "MyEdge.h"
#include "Settings.h"
#include "core_file.h"
#include "mining_utils.h"
#include "Signature.h"

char Signature::sig_buffer[10000];

int mysprintf(char *str, std::string x) {
    int magnitude = 0;
    for (int i = 0; i < x.size(); i++) {
        *(str + i) = x[i];
    }
    magnitude += x.size();
    return magnitude;
}

void print_combinations(std::vector<std::vector<NodeInfo *> *> *all) {
    for (auto combination: *all) {
        for (auto n_info: *combination) {
            std::cout << n_info->node->get_id() << ", ";
        }
        std::cout << std::endl;
    }
}

std::string Sig_Partition::get_prefix() {
    std::string prefix;
    for (auto &node: nodes) {
        prefix += (node->node->get_label() + "\0");
    }

    return prefix;
}

void Sig_Partition::sig_part_destructor() {
    for (auto temp_v: combinations) {
        delete temp_v;
    }
}

bool Signature::sort_partitions(std::vector<Sig_Partition *> &parts) {
    bool b = true;
    sort(parts.begin(), parts.begin() + parts.size(), descending);
    int i = 0;
    for (auto part: parts) {
        if (part->get_id() != i)
            b = false;
        part->set_id(i++);
    }
    return b;
}

std::string Signature::generate(MyGraph *graph) {
    if (graph->get_nodes_num() == 0)
        return "";

    std::map<int, NodeInfo *> all_nodes;

    std::map<std::string, Sig_Partition *> parts;
    for (std::tr1::unordered_map<int, MyNode *>::const_iterator enum1 = graph->get_nodes_iterator(); enum1 !=
                                                                                                     graph->get_nodes_end_iterator(); ++enum1) {
        NodeInfo *nInfo = new NodeInfo((*enum1).second);
        std::string label = nInfo->node->get_label();
        int degree = nInfo->node->get_edges_size();
        std::string key = std::to_string(degree) + "_" + label;

        auto t_iter = parts.find(key);
        Sig_Partition *part;
        if (t_iter == parts.end()) {
            part = new Sig_Partition(parts.size());
            parts[key] = part;
        } else {
            part = (*t_iter).second;
        }

        part->add_node(nInfo);
        nInfo->part_id = part->get_id();
        all_nodes[nInfo->node->get_id()] = nInfo;
        part->set_sorting_values();
    }

    std::vector<Sig_Partition *> parts_v;
    map_to_vec(parts, parts_v);
    parts.clear();
    sort_partitions(parts_v);
    for (auto part: parts_v) {
        for (auto enum2 = part->get_nodes_enum(); enum2 != part->get_nodes_end(); ++enum2) {
            NodeInfo *n_info = *enum2;
            n_info->nl = generate_neighbors_list(n_info, all_nodes);
        }
        part->sort_nodes();
    }
    sort_partitions(parts_v);

    while (true) {
        bool b = false;
        unsigned int i = 0;
        for (auto enum1 = parts_v.begin(); enum1 != parts_v.end(); ++enum1) {
            Sig_Partition *part = *enum1;
            std::map<std::string, Sig_Partition *> *new_parts = part->get_new_parts();
            if (new_parts != nullptr) {
                enum1 = parts_v.end();

                std::vector<Sig_Partition *> new_parts_arr;
                map_to_vec(*new_parts, new_parts_arr);
                new_parts->clear();
                delete new_parts;
                for (auto &j: new_parts_arr) {
                    j->set_sorting_values();
                }

                sort(new_parts_arr.begin(), new_parts_arr.begin() + new_parts_arr.size(), ascending);

                (*(parts_v.begin() + i))->sig_part_destructor();
                delete (*(parts_v.begin() + i));
                parts_v.erase(parts_v.begin() + i);

                int j_count = 0;
                for (auto j: new_parts_arr) {
                    j->set_id(i + j_count);
                    j->set_sorting_values();
                    parts_v.insert(parts_v.begin() + i + (j_count++), j);
                }

                for (unsigned int j = i + new_parts_arr.size(); j < parts_v.size(); j++) {
                    (*(parts_v.begin() + j))->set_id(j);
                }
                new_parts_arr.clear();

                for (auto part2: parts_v) {
                    int toto = part2->get_id();

                    for (auto enum2 = part2->get_nodes_enum(); enum2 != part2->get_nodes_end(); ++enum2) {
                        NodeInfo *nInfo = *enum2;
                        nInfo->nl = generate_neighbors_list(nInfo, all_nodes);
                    }
                    part2->sort_nodes();
                }

                b = true;

                break;
            }
            i++;
        }

        if (i == parts_v.size()) break;
    }

    std::string prefix;
    for (auto &i: parts_v) {
        if (enable_print) {
            std::cout << "Part:" + int_to_string(i->get_id()) << std::endl;
            std::cout << i->get_combinations()->size() << std::endl;
        }
        i->counter = 0;
        prefix += i->get_prefix();
        prefix += ",";
    }

    int max_label_id = 0;
    for (std::tr1::unordered_map<int, MyNode *>::const_iterator iter = graph->get_nodes_iterator(); iter !=
                                                                                                    graph->get_nodes_end_iterator(); ++iter) {
        int curr_id = iter->second->get_id();
        if (max_label_id < curr_id)
            max_label_id = curr_id;
    }

    std::string **lookup = new std::string *[max_label_id + 1];
    for (int i = 0; i < (max_label_id + 1); i++)
        lookup[i] = new std::string[max_label_id + 1];
    for (std::tr1::unordered_map<int, MyNode *>::const_iterator i = graph->get_nodes_iterator(); i !=
                                                                                                 graph->get_nodes_end_iterator(); i++) {
        int id1 = i->second->get_id();
        for (std::tr1::unordered_map<int, MyNode *>::const_iterator ii = graph->get_nodes_iterator(); ii !=
                                                                                                      graph->get_nodes_end_iterator(); ii++) {
            int id2 = ii->second->get_id();
            lookup[id1][id2] = graph->get_edge_label(id1, id2);
        }
    }

    for (int i = 0; i < parts_v.size(); i++) {
        if (parts_v.at(i)->get_num_nodes() > 10) {
            if (Settings::debug_msg) {
                std::cout << "pre_pre_ number of combinations is very large = (" << parts_v.at(i)->get_num_nodes()
                          << "!), "
                          << std::endl;
            }
            std::stringstream nosig;
            nosig << "X" << graph->get_nodes_num() << "_" << graph->get_edges_num();
            return nosig.str();
        }
    }

    long number_of_combinations_ = 1;
    for (int i = 1; i < parts_v.size(); i++) {
        number_of_combinations_ = number_of_combinations_ * parts_v.at(i)->get_combinations()->size();
    }

    if (number_of_combinations_ > 2000000 || parts_v.at(0)->get_combinations()->size() > 200000) {
        if (Settings::debug_msg) {
            std::cout << "pre_ number of combinations exceeded: " << number_of_combinations_ << ", " << parts_v.at(
                    0)->get_combinations()->size() << std::endl;
        }

        std::stringstream nosig;
        nosig << "X" << graph->get_nodes_num() << "_" << graph->get_edges_num();
        return nosig.str();
    }

    char *max_sig_line = 0;
    Sig_Partition *first_part = parts_v.at(0);
    for (auto single_comb: *first_part->get_combinations()) {
        char *temp1 = generate_can_label(single_comb, graph, lookup, true);

        if (max_sig_line == 0 || strcmp(temp1, max_sig_line) > 0) {
            if (max_sig_line != 0)
                delete[] max_sig_line;

            int length = strlen(temp1);
            max_sig_line = new char[length + 1];
            strcpy(max_sig_line, temp1);
        }
    }

    for (auto iter = first_part->get_combinations()->begin(); iter != first_part->get_combinations()->end();) {
        std::vector<NodeInfo *> *single_comb = (*iter);

        char *temp1 = generate_can_label(single_comb, graph, lookup, true);

        if (strcmp(temp1, max_sig_line) < 0) {
            iter = first_part->get_combinations()->erase(iter);
        } else
            iter++;
    }

    long number_of_combinations = 1;
    for (int i = 0; i < parts_v.size(); i++) {
        number_of_combinations = number_of_combinations * parts_v.at(i)->get_combinations()->size();
    }

    if (number_of_combinations > 20000000) {
        if (Settings::debug_msg) {
            std::cout << "number of combinations exceeded: " << number_of_combinations_ << ", " << parts_v.at(
                    0)->get_combinations()->size() << std::endl;
        }

        std::stringstream nosig;
        nosig << "X" << graph->get_nodes_num() << "_" << graph->get_edges_num();
        return nosig.str();
    }

    char *max_sig = new char[1];
    max_sig[0] = 0;

    std::vector<NodeInfo *> *best_combination = nullptr;
    bool b = true;
    int count = 0;
    while (true) {
        count++;
        if (count % 1000000 == 0) {
            std::cout << count << "/" << number_of_combinations << std::endl;
        }
        auto *single_combination = new std::vector<NodeInfo *>();
        for (auto &i: parts_v) {
            std::vector<NodeInfo *> *temp = *((i->get_combinations())->begin() + i->counter);
            single_combination->insert(single_combination->end(), temp->begin(), temp->end());
        }

        char *temp = generate_can_label(single_combination, graph, lookup);

        if (enable_print) std::cout << temp << std::endl;
        if (best_combination == 0 || strcmp(temp, max_sig) > 0) {
            delete[] max_sig;
            delete best_combination;

            int sigLength = strlen(temp);
            max_sig = new char[sigLength + 1];
            strcpy(max_sig, temp);

            best_combination = single_combination;
        } else {
            delete single_combination;
        }

        int i_counter = parts_v.size() - 1;
        for (auto i = parts_v.rbegin(); i != parts_v.rend(); ++i, --i_counter) {
            if ((*i)->counter < (*i)->get_combinations()->size() - 1) {
                (*i)->counter++;
                break;
            } else if (i_counter == 0)
                b = false;
            else
                (*i)->counter = 0;
        }

        if (!b)
            break;
    }

    for (int i = 0; i < (max_label_id + 1); i++)
        delete[] lookup[i];
    delete[] lookup;


    if (enable_print) std::cout << "Start destruction ...." << std::endl;
    for (auto i: parts_v) {
        if (i->get_combinations()->size() > 0) {
            for (auto iter1 = (*i->get_combinations()->begin())->begin();
                 iter1 != (*i->get_combinations()->begin())->end(); ++iter1)
                delete (*iter1);
        }
    }
    parts_v.clear();
    if (enable_print) std::cout << "All destruction work finished" << std::endl;

    std::string temp(prefix);
    temp.append(max_sig);
    delete max_sig;
    return temp;
}

std::string Signature::generate_neighbors_list(NodeInfo *nInfo, std::map<int, NodeInfo *> &allNodes) {
    std::string sb = "";

    MyNode *node = nInfo->node;

    std::vector<PartID_label *> ordered_nl;
    for (std::tr1::unordered_map<int, void *>::const_iterator enum1 = node->get_edges_begin(); enum1 !=
                                                                                               node->get_edges_end(); ++enum1) {
        MyEdge *edge = (MyEdge *) enum1->second;

        MyNode *o_node = edge->get_neighbor();

        PartID_label *pidl = new PartID_label((allNodes.find(o_node->get_id()))->second->part_id,
                                              o_node->get_label() + edge->get_label());

        auto i = ordered_nl.begin();
        for (; i != ordered_nl.end(); i++) {
            PartID_label *current_pidl = *i;
            if (!(pidl->part_id > current_pidl->part_id ||
                  (pidl->part_id == current_pidl->part_id && pidl->label.compare(current_pidl->label) > 0)))
                break;
        }
        ordered_nl.insert(i, pidl);
    }

    for (auto &i: ordered_nl) {
        sb.append(i->to_string());
        delete i;
    }

    return sb;
}

char *
Signature::generate_can_label(std::vector<NodeInfo *> *nodes, MyGraph *graph, std::string **ellt, bool one_line_only) {
    char *sb = Signature::sig_buffer;
    char *start_sb = sb;

    auto nodes_end = nodes->end();
    std::vector<NodeInfo *>::iterator j;
    auto enum1 = nodes->begin();
    while (true) {
        j = enum1 + 1;
        if (j == nodes_end)
            break;

        std::string *_label = ellt[(*enum1)->node->get_id()];

        for (; j != nodes_end; ++j) {
            std::string label = _label[(*j)->node->get_id()];
            if (label == "") {
                *(sb++) = '0';
            } else {
                sb += mysprintf(sb, label);
            }
            *(sb++) = '*';
        }
        *(sb++) = ',';

        if (one_line_only)
            break;
        ++enum1;
    }

    *sb = '\0';
    return start_sb;
}

bool ascending(Sig_Partition *a, Sig_Partition *b) {
    return !descending(a, b);
}

bool descending(Sig_Partition *a, Sig_Partition *b) {
    int r = a->node_label.compare(b->node_label);
    if (r != 0) {
        if (r < 0) return false;
        else return true;
    }

    if (a->degree != b->degree) {
        if (a->degree - b->degree < 0) return false;
        else return true;
    }

    r = a->nl.compare(b->nl);
    if (r != 0) {
        if (r < 0) return false;
        else return true;
    }

    if (a->part_id < b->part_id)
        return false;
    else
        return true;
}

/**
 * this function should be called once all partitions get properly partitioned
 */
void Sig_Partition::set_sorting_values() {
    NodeInfo *n_info = *(nodes.begin());
    this->degree = n_info->node->get_edges_size();
    this->node_label = n_info->node->get_label();
    this->nl = n_info->nl;
}

Sig_Partition::Sig_Partition(int part_id) {
    this->part_id = part_id;
}

void Sig_Partition::add_node(NodeInfo *n_info) {
    n_info->part_id = this->part_id;
    this->nodes.push_back(n_info);
}

std::vector<NodeInfo *>::const_iterator Sig_Partition::get_nodes_enum() const {
    return this->nodes.begin();
}

int Sig_Partition::get_num_nodes() {
    return this->nodes.size();
}

void Sig_Partition::clear_nodes() {
    this->nodes.clear();
}

void Sig_Partition::set_id(int partID) {
    this->part_id = partID;
    for (auto nInfo: this->nodes) {
        nInfo->part_id = this->part_id;
    }
}

int Sig_Partition::get_id() {
    return this->part_id;
}

std::map<std::string, Sig_Partition *> *Sig_Partition::get_new_parts() {
    if (this->nodes.size() == 1)
        return nullptr;

    auto *parts = new std::map<std::string, Sig_Partition *>();
    for (auto nInfo: nodes) {
        std::string key = nInfo->nl;

        auto temp = parts->find(key);
        Sig_Partition *part;
        if (temp == parts->end()) {
            part = new Sig_Partition(parts->size());
            (*parts)[key] = part;
        } else
            part = temp->second;
        part->add_node(nInfo);
    }

    if (parts->size() == 1) {
        parts->begin()->second->sig_part_destructor();
        delete parts->begin()->second;
        parts->clear();
        delete parts;
        return nullptr;
    } else
        return parts;
}

std::string Sig_Partition::to_string() {
    std::string sb = "Partition ID: " + int_to_string(part_id) + "\n";

    for (auto n_info: nodes) {
        sb += (n_info->to_string() + "\n");
    }

    return sb;
}

std::vector<std::vector<NodeInfo *> *> *Sig_Partition::get_combinations() {
    if (this->combinations.size() > 0)
        return &(this->combinations);

    auto *notused = new std::vector<NodeInfo *>(nodes);

    std::tr1::unordered_set<int> all_nodes;
    for (auto &node: nodes) {
        all_nodes.insert(node->node->get_id());
    }
    bool connected_within = true;
    for (auto &iter: nodes) {
        MyNode *node = iter->node;
        for (std::tr1::unordered_map<int, void *>::iterator iter1 = node->get_edges_begin(); iter1 !=
                                                                                             node->get_edges_end(); iter1++) {
            if (all_nodes.find(((MyEdge *) iter1->second)->get_neighbor()->get_id()) == all_nodes.end()) {
                connected_within = false;
                break;
            }
        }
        if (!connected_within)
            break;
    }

    combinations_fn(notused, connected_within);

    delete notused;

    return &(this->combinations);
}

void Sig_Partition::combinations_fn(std::vector<NodeInfo *> *notused, bool connected_within) {
    do {
        this->combinations.insert(this->combinations.end(), new std::vector<NodeInfo *>(*notused));
        if (connected_within)
            break;
    } while (std::next_permutation(notused->begin(), notused->end(), node_info_des));
}

void Sig_Partition::sort_nodes() {
    sort(nodes.begin(), nodes.begin() + nodes.size(), node_info_des);
}

bool node_info_asc(NodeInfo *a, NodeInfo *b) {
    return !node_info_des(a, b);
}

bool node_info_des(NodeInfo *a, NodeInfo *b) {
    int r = a->nl.compare(b->nl);
    if (r < 0)
        return false;
    else if (r == 0) {
        if (a->node->get_id() < b->node->get_id())
            return false;
        else
            return true;
    } else
        return true;
}

NodeInfo::NodeInfo(MyNode *node) {
    this->node = node;
}

std::string NodeInfo::to_string() {
    return "NodeID: " + int_to_string(this->node->get_id()) + "\nPartID: " + int_to_string(this->part_id) +
           "\nNeighbors List:" +
           this->nl;
}

std::string PartID_label::to_string() {
    return "(p" + int_to_string(this->part_id) + "," + this->label + "),";
}

/**
 * A function to copy map content into a vector
 */
void map_to_vec(std::map<std::string, Sig_Partition *> &m, std::vector<Sig_Partition *> &v) {
    for (auto it: m) {
        v.push_back(it.second);
    }
}

