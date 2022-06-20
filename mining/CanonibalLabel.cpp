/**
 * This class is responsible for generating a unique graph canonical label
 */

#include "EdgeX.h"
#include "Settings.h"
#include "core_file.h"
#include "mining_utils.h"
#include "CanonicalLabel.h"

char CanonicalLabel::sig_buffer[10000];

int mysprintf(char *str, std::string x) {
    int magnitude = 0;
    for (int i = 0; i < x.size(); i++) {
        *(str + i) = x[i];
    }
    magnitude += x.size();
    return magnitude;
}

int mysprintf(char *str, int x) {
    int i;
    int magnitude = 0;
    if (x < 10) {
        // change the int 'x' to char, and put in str
        *str = x + '0';
        // move the pointer str to the next space
        return 1;
    } else if (x < 100) {
        // Convert x to two-digit character form, and put in str
        *(str + 1) = (x % 10) + '0';
        *(str) = (x / 10) + '0';
        // Move the pointer back two paces
        return 2;
    } else {
        int tmp = x;
        while (tmp > 0) {
            tmp /= 10;
            magnitude++;
        }
        i = magnitude - 1;
    }

    while (x > 0) {
        *(str + i) = (x % 10) + '0';
        x = x / 10;
        i = i - 1;
    }

    return magnitude;
}

void print_combinations(std::vector<std::vector<NodeWithInfo *> *> *all) {
    for (std::vector<std::vector<NodeWithInfo *> *>::iterator enum1 = all->begin(); enum1 != all->end(); ++enum1) {
        std::vector<NodeWithInfo *> *combination = *enum1;
        for (std::vector<NodeWithInfo *>::iterator enum11 = combination->begin(); enum11 != combination->end(); ++enum11) {
            NodeWithInfo *n_info = *enum11;
            std::cout << n_info->node->get_id() << ", ";
        }
        std::cout << std::endl;
    }
}


std::string CL_Partition::get_prefix() {
    std::string prefix;
    for (std::vector<NodeWithInfo *>::iterator iter = nodes.begin(); iter != nodes.end(); iter++) {
        prefix.append((*iter)->node->get_label());
        prefix.append("\0");
    }

    return prefix;
}

void CL_Partition::cl_part_destructor() {
    //delete combinations
    for (std::vector<std::vector<NodeWithInfo *> *>::iterator iter = combinations.begin(); iter != combinations.end(); ++iter) {
        std::vector<NodeWithInfo *> *temp_v = (*iter);

        delete temp_v;
    }
}

bool CanonicalLabel::sort_partitions(std::vector<CL_Partition *> &parts) {
    bool b = true;
    // Sort by node_label in descending order
    sort(parts.begin(), parts.begin() + parts.size(), descending);//std::greater<CL_Partition*>());
    int i = 0;
    for (auto part: parts) {
        int old_id = part->get_id();
        if (old_id != i)
            b = false;
        part->set_id(i);
        i++;
    }
    return b;
}

/**
 * the main function for generating canonical label for the given graph
 * This function is based on the work published in:
 * "Finding Frequent Patterns in a Large Sparse Graph", DMKD 2005
 * If the graph is huge and takes much time, the returned canonical label is not unique and is identified by prefix of X
 */
std::string CanonicalLabel::generate(GraphX *graph) {
    if (graph->get_num_of_nodes() == 0)
        return "";

    std::map<int, NodeWithInfo *> all_nodes;

    //partition by label and degree, which means: "degree_label" -> group of nodes' info
    std::map<std::string, CL_Partition *> parts;
    for (std::tr1::unordered_map<int, NodeX *>::const_iterator enum1 = graph->get_nodes_iterator(); enum1 !=
                                                                                               graph->get_nodes_end_iterator(); ++enum1) {
        NodeX *node = (*enum1).second;
        NodeWithInfo *nInfo = new NodeWithInfo(node);
        std::string label = nInfo->node->get_label();
        int degree = nInfo->node->get_edges_size();
        std::string key = degree + "_" + label;

        std::map<std::string, CL_Partition *>::iterator t_iter = parts.find(key);
        CL_Partition *part;
        if (t_iter == parts.end()) {
            part = new CL_Partition(parts.size());
            parts[key] = part;
        } else {
            part = (*t_iter).second;
        }

        part->add_node(nInfo);
        nInfo->part_id = part->get_id();
        all_nodes[nInfo->node->get_id()] = nInfo;
        //cout<<"nInfo ID: "<<nInfo->node->get_id()<<", nInfo->part_id: "<<nInfo->part_id<<endl;
        part->set_sorting_values();
    }

    // convert the map "parts" to the vector "parts_v"
    std::vector<CL_Partition *> parts_v;
    // parts_v: the vector of CL_Partition
    map_to_vec(parts, parts_v);
    parts.clear();
    // Sort by node_label in descending order, reorder the part_id
    sort_partitions(parts_v);
    //generate Neighbors list, then sort nodes inside each partition
    for (auto part: parts_v) {
        //for each node generate NL
        for (auto enum2 = part->get_nodes_enum(); enum2 != part->get_nodes_end(); ++enum2) {
            NodeWithInfo *n_info = *enum2;
            // otherNodesPartID + {otherNodeLabel + edge_label}
            n_info->nl = generate_neighbors_list(n_info, all_nodes);
            //cout<<"NInfo Node ID: "<<n_info->to_std::string()<<endl;
        }
        //sort nodes
        part->sort_nodes();
    }
    sort_partitions(parts_v);

    //iterative partitioning
    while (true) {
        //generate new partitions
        bool b = false;
        unsigned int i = 0;
        for (auto enum1 = parts_v.begin(); enum1 != parts_v.end(); ++enum1) {
            CL_Partition *part = *enum1;
            std::map<std::string, CL_Partition *> *new_parts = part->get_new_parts();
            if (new_parts != NULL) {
                //invalidate the 'enum1' iterator
                enum1 = parts_v.end();

                std::vector<CL_Partition *> new_parts_arr;
                map_to_vec(*new_parts, new_parts_arr);
                new_parts->clear();
                delete new_parts;
                for (auto &j: new_parts_arr) {
                    j->set_sorting_values();
                }

                sort(new_parts_arr.begin(), new_parts_arr.begin() + new_parts_arr.size(), ascending);

                (*(parts_v.begin() + i))->cl_part_destructor();
                delete (*(parts_v.begin() + i));
                parts_v.erase(parts_v.begin() + i);

                int j_count = 0;
                //Repartition using partition information in parts_v and new_parts_arr
                for (auto j = new_parts_arr.begin(); j != new_parts_arr.end(); ++j, j_count++) {
                    (*j)->set_id(i + j_count);
                    (*j)->set_sorting_values();
                    parts_v.insert(parts_v.begin() + i + j_count, *j);
                }

                for (unsigned int j = i + new_parts_arr.size(); j < parts_v.size(); j++) {
                    (*(parts_v.begin() + j))->set_id(j);
                }
                new_parts_arr.clear();

                //generate Neighbors list, then sort nodes inside each partition
                for (auto part2: parts_v) {
                    int toto = part2->get_id();

                    //for each node generate NL
                    for (auto enum2 = part2->get_nodes_enum(); enum2 != part2->get_nodes_end(); ++enum2) {
                        NodeWithInfo *nInfo = *enum2;
                        nInfo->nl = generate_neighbors_list(nInfo, all_nodes);
                    }
                    //sort nodes
                    part2->sort_nodes();
                }

                b = true;

                break;
            }
            i++;
        }

        if (i == parts_v.size()) break;
    }

    //reset counters and generate signature prefix
    std::string prefix;
    for (auto &i: parts_v) {
        if (enable_print) {
            std::cout << "Part:" + int_to_string(i->get_id()) << std::endl;
            std::cout << i->get_combinations()->size() << std::endl;
        }
        i->counter = 0;
        prefix.append(i->get_prefix());
        prefix.append(",");
    }

    //generate a fast lookup array for edge labels
    //get the maximum label id
    int max_label_id = 0;
    for (std::tr1::unordered_map<int, NodeX *>::const_iterator iter = graph->get_nodes_iterator(); iter !=
                                                                                              graph->get_nodes_end_iterator(); ++iter) {
        int curr_id = iter->second->get_id();
        if (max_label_id < curr_id)
            max_label_id = curr_id;
    }

//    ellt: edge label look table
    std::string **ellt = new std::string *[max_label_id + 1];
    for (int i = 0; i < (max_label_id + 1); i++)
        ellt[i] = new std::string[max_label_id + 1];
    //fill in the edge labels
    for (std::tr1::unordered_map<int, NodeX *>::const_iterator i = graph->get_nodes_iterator(); i !=
                                                                                           graph->get_nodes_end_iterator(); i++) {
        int id1 = i->second->get_id();
        for (std::tr1::unordered_map<int, NodeX *>::const_iterator ii = graph->get_nodes_iterator(); ii !=
                                                                                                graph->get_nodes_end_iterator(); ii++) {
            int id2 = ii->second->get_id();
            ellt[id1][id2] = graph->get_edge_label(id1, id2);
        }
    }

    for (int i = 0; i < parts_v.size(); i++) {
        if (parts_v.at(i)->get_num_nodes() > 10) {
            if (Settings::debug_msg) {
                std::cout << "pre_pre_ number of combinations is very large = (" << parts_v.at(i)->get_num_nodes() << "!), "
                     << std::endl;
            }
            std::stringstream nosig;
            nosig << "X" << graph->get_num_of_nodes() << "_" << graph->get_num_of_edges();
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
        nosig << "X" << graph->get_num_of_nodes() << "_" << graph->get_num_of_edges();
        return nosig.str();
    }

    //an optimization for the first partition
    //only store permutations where the first line is the max

    //get the max signature line
    char *max_sig_line = 0;
    CL_Partition *first_part = parts_v.at(0);
    for (auto single_comb: *first_part->get_combinations()) {
        char *temp1 = generate_can_label(single_comb, graph, ellt, true);

        if (max_sig_line == 0 || strcmp(temp1, max_sig_line) > 0) {
            if (max_sig_line != 0)
                delete[] max_sig_line;

            int length = strlen(temp1);
            max_sig_line = new char[length + 1];
            strcpy(max_sig_line, temp1);
        }
    }

    //delete any permutation from the first partition that is less than the max
    for (auto iter = first_part->get_combinations()->begin(); iter != first_part->get_combinations()->end();) {
        std::vector<NodeWithInfo *> *single_comb = (*iter);

        char *temp1 = generate_can_label(single_comb, graph, ellt, true);

        if (strcmp(temp1, max_sig_line) < 0) {
            iter = first_part->get_combinations()->erase(iter);
        } else
            iter++;
    }

    long number_of_combinations = 1;
    for (int i = 0; i < parts_v.size(); i++) {
        number_of_combinations = number_of_combinations * parts_v.at(i)->get_combinations()->size();
    }

    if (number_of_combinations > 20000000)//number_of_combinations>2)//it was 20000000
    {
        if (Settings::debug_msg) {
            std::cout << "number of combinations exceeded: " << number_of_combinations_ << ", " << parts_v.at(
                    0)->get_combinations()->size() << std::endl;
        }

        std::stringstream nosig;
        nosig << "X" << graph->get_num_of_nodes() << "_" << graph->get_num_of_edges();
        return nosig.str();
    }

    //generate permutations and save the max
    char *max_cl = new char[1];
    max_cl[0] = 0;

    std::vector<NodeWithInfo *> *best_combination = nullptr;
    bool b = true;
    int count = 0;
    while (true) {
        count++;
        if (count % 1000000 == 0) {
            std::cout << count << "/" << number_of_combinations << std::endl;
        }
        //add a combination
        std::vector<NodeWithInfo *> *single_combination = new std::vector<NodeWithInfo *>();
        for (auto &i: parts_v) {
            std::vector<NodeWithInfo *> *temp = *((i->get_combinations())->begin() + i->counter);
            single_combination->insert(single_combination->end(), temp->begin(), temp->end());
        }

        char *temp = generate_can_label(single_combination, graph, ellt);

        if (enable_print) std::cout << temp << std::endl;
        if (best_combination == 0 || strcmp(temp, max_cl) > 0) {
            delete[] max_cl;
            delete best_combination;

            int sigLength = strlen(temp);
            max_cl = new char[sigLength + 1];
            strcpy(max_cl, temp);

            best_combination = single_combination;
        } else {
            //delete temp;
            delete single_combination;
        }

        //increase counters
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
        delete[] ellt[i];
    delete[] ellt;

    //cout<<"*"<<max_cl<<endl;

    if (enable_print) std::cout << "Start destruction ...." << std::endl;
    for (std::vector<CL_Partition *>::iterator i = parts_v.begin(); i != parts_v.end(); ++i) {
        //added to delete NodeWithInfo for this partition
        if ((*i)->get_combinations()->size() > 0) {
            for (auto iter1 = (*(*i)->get_combinations()->begin())->begin();
                 iter1 != (*(*i)->get_combinations()->begin())->end(); ++iter1) {
                delete (*iter1);
            }
        }

        (*i)->cl_part_destructor();
        delete (*i);
    }
    parts_v.clear();
    if (enable_print) std::cout << "All destruction work finished" << std::endl;

    std::string temp(prefix);
    temp.append(max_cl);
    delete max_cl;
    return temp;
}

std::string CanonicalLabel::generate_neighbors_list(NodeWithInfo *nInfo, std::map<int, NodeWithInfo *> &allNodes) {
    std::string sb = "";

    NodeX *node = nInfo->node;

    std::vector<PartID_label *> ordered_nl;
    for (std::tr1::unordered_map<int, void *>::const_iterator enum1 = node->get_edges_iterator(); enum1 !=
                                                                                             node->get_edges_end_iterator(); ++enum1) {
        EdgeX *edge = (EdgeX *) enum1->second;

        //get the other node partition
        NodeX *o_node = edge->get_other_node();
        int o_node_part_id = (allNodes.find(o_node->get_id()))->second->part_id;

        //prepare the details
        std::string o_node_label = o_node->get_label();
        PartID_label *pidl = new PartID_label();
        // pidl->label: otherNodeLabel + edge_label
        pidl->label = o_node_label;
        pidl->label = pidl->label + edge->get_label();//added by ehab on 13 Aug 2015
        pidl->part_id = o_node_part_id;

        //put the details in order
        auto i = ordered_nl.begin();
        for (; i != ordered_nl.end(); i++) {
            PartID_label *current_pidl = *i;
            if (pidl->part_id > current_pidl->part_id ||
                (pidl->part_id == current_pidl->part_id && pidl->label.compare(current_pidl->label) > 0));
            else
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
CanonicalLabel::generate_can_label(std::vector<NodeWithInfo *> *nodes, GraphX *graph, std::string **ellt, bool one_line_only) {
    char *sb = CanonicalLabel::sig_buffer;//new char[(nodes->size()-1)*10];
    char *start_sb = sb;

    //add the upper triangle thing
    auto nodes_end = nodes->end();
    std::vector<NodeWithInfo *>::iterator j;
    auto enum1 = nodes->begin();
    while (true)
        //for(vector<NodeWithInfo* >::iterator enum1 = nodes->begin();enum1!=nodes_end;++enum1)
    {
        NodeWithInfo *n_info1 = *enum1;
        std::string *_label = ellt[n_info1->node->get_id()];

        j = enum1 + 1;
        if (j == nodes_end)
            break;
        for (; j != nodes_end; ++j) {
            // edge label
            std::string label = _label[(*j)->node->get_id()];
            if (label == "") {
                *sb = '0';
                sb++;
            } else {
                sb += mysprintf(sb, label);
            }
            *sb = '*';
            sb++;
        }
        *sb = ',';
        sb++;

        if (one_line_only)
            break;
        ++enum1;
    }

    *sb = '\0';
    return start_sb;
}

bool ascending(CL_Partition *a, CL_Partition *b) {
    int r = a->node_label.compare(b->node_label);
    if (r != 0) {
        if (r < 0) return true;
        else return false;
    }

    if (a->degree != b->degree) {
        if (a->degree - b->degree < 0) return true;
        else return false;
    }

    r = a->nl.compare(b->nl);
    if (r != 0) {
        if (r < 0) return true;
        else return false;
    }

    if (a->part_id < b->part_id)
        return true;
    else
        return false;
}

bool descending(CL_Partition *a, CL_Partition *b) {
    return !ascending(a, b);
}

/**
 * this function should be called once all partitions get properly partitioned
 */
void CL_Partition::set_sorting_values() {
    NodeWithInfo *n_info = *(nodes.begin());
    degree = n_info->node->get_edges_size();
    node_label = n_info->node->get_label();
    nl = n_info->nl;
}

CL_Partition::CL_Partition(int part_id) {
    this->part_id = part_id;
}

void CL_Partition::add_node(NodeWithInfo *n_info) {
    n_info->part_id = this->part_id;
    nodes.push_back(n_info);
}

std::vector<NodeWithInfo *>::const_iterator CL_Partition::get_nodes_enum() const {
    return nodes.begin();
}

int CL_Partition::get_num_nodes() {
    return nodes.size();
}

void CL_Partition::clear_nodes() {
    nodes.clear();
}

void CL_Partition::set_id(int partID) {
    this->part_id = partID;
    for (std::vector<NodeWithInfo *>::iterator enum1 = nodes.begin(); enum1 != nodes.end(); ++enum1) {
        NodeWithInfo *nInfo = *enum1;
        nInfo->part_id = this->part_id;
    }
}

int CL_Partition::get_id() {
    return part_id;
}

/**
 * get the partitioning of this partition
 * returns null if no need to partition
 * [Translated to C++]
 */
std::map<std::string, CL_Partition *> *CL_Partition::get_new_parts() {
    if (nodes.size() == 1)
        return NULL;

    auto *parts = new std::map<std::string, CL_Partition *>();
    for (auto nInfo: nodes) {
        std::string key = nInfo->nl;

        auto temp = parts->find(key);
        CL_Partition *part;
        if (temp == parts->end()) {
            part = new CL_Partition(parts->size());
            (*parts)[key] = part;
        } else
            part = temp->second;
        part->add_node(nInfo);
    }

    if (parts->size() == 1) {
        parts->begin()->second->cl_part_destructor();
        delete parts->begin()->second;
        parts->clear();
        delete parts;
        return NULL;
    } else
        return parts;
}

std::string CL_Partition::to_string() {
    std::string sb = "";
    sb.append("Partition ID: " + int_to_string(part_id) + "\n");

    for (std::vector<NodeWithInfo *>::iterator enum1 = nodes.begin(); enum1 != nodes.end(); ++enum1) {
        NodeWithInfo *n_info = *enum1;
        sb.append(n_info->to_string() + "\n");
    }

    return sb;
}

std::vector<std::vector<NodeWithInfo *> *> *CL_Partition::get_combinations() {
    if (combinations.size() > 0)
        return &combinations;

    auto *notused = new std::vector<NodeWithInfo *>(nodes);

    //check if all nodes within this partition are only connected to each other
    //collect all nodes in a map
    std::tr1::unordered_set<int> all_nodes;
    for (auto &node: nodes) {
        all_nodes.insert(node->node->get_id());
    }
    bool connected_within = true;
    for (auto &iter: nodes) {
        NodeX *node = iter->node;
        for (std::tr1::unordered_map<int, void *>::iterator iter1 = node->get_edges_iterator(); iter1 !=
                                                                                           node->get_edges_end_iterator(); iter1++) {
            int other_node_id = ((EdgeX *) iter1->second)->get_other_node()->get_id();
            if (all_nodes.find(other_node_id) == all_nodes.end()) {
                connected_within = false;
                break;
            }
        }
        if (!connected_within)
            break;
    }

    combinations_fn(notused, connected_within);

    delete notused;

    return &combinations;
}

void CL_Partition::combinations_fn(std::vector<NodeWithInfo *> *notused, bool connected_within) {
    do {
        auto *v1 = new std::vector<NodeWithInfo *>(*notused);
        combinations.insert(combinations.end(), v1);
        //this needs a proof
        if (connected_within)
            break;
    } while (std::next_permutation(notused->begin(), notused->end(), NodeWithInfo_descending));
}

void CL_Partition::sort_nodes() {
    sort(nodes.begin(), nodes.begin() + nodes.size(), NodeWithInfo_descending);//, std::greater<NodeWithInfo>());
}

bool NodeWithInfo_ascending(NodeWithInfo *a, NodeWithInfo *b) {
    int r = a->nl.compare(b->nl);
    if (r < 0)
        return true;
    else if (r == 0)//this part is not needed for the algorithm, but it is needed for the STL::next_permutation function
    {
        if (a->node->get_id() < b->node->get_id())
            return true;
        else
            return false;
    } else
        return false;
}

bool NodeWithInfo_descending(NodeWithInfo *a, NodeWithInfo *b) {
    return !NodeWithInfo_ascending(a, b);
}

NodeWithInfo::NodeWithInfo(NodeX *node) {
    this->node = node;
}

std::string NodeWithInfo::to_string() {
    return "NodeID: " + int_to_string(node->get_id()) + "\nPartID: " + int_to_string(part_id) + "\nNeighbors List:" +
           nl;
}

std::string PartID_label::to_string() {
    return "(p" + int_to_string(part_id) + "," + label + "),";
}

/**
 * A function to copy map content into a vector
 */
void map_to_vec(std::map<std::string, CL_Partition *> &m, std::vector<CL_Partition *> &v) {
    for (std::map<std::string, CL_Partition *>::const_iterator it = m.begin(); it != m.end(); ++it) {
        v.push_back(it->second);
    }
}

