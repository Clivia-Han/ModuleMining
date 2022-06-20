/**
 * This class is responsible for generating a unique graph canonical label
 */
#pragma once

#include "GraphX.h"
#include "core_file.h"

class NodeWithInfo {
public:
    NodeX *node;
    std::string nl;//neighbors list
    int part_id;

    NodeWithInfo(NodeX *node);

    ~NodeWithInfo() = default;

    std::string to_string();

    bool operator<(const NodeWithInfo &) const;

    friend bool NodeWithInfo_ascending(NodeWithInfo *a, NodeWithInfo *b);

    friend bool NodeWithInfo_descending(NodeWithInfo *a, NodeWithInfo *b);

};

bool NodeWithInfo_ascending(NodeWithInfo *a, NodeWithInfo *b);

bool NodeWithInfo_descending(NodeWithInfo *a, NodeWithInfo *b);

class CL_Partition {
private:
    int part_id;
    std::vector<NodeWithInfo *> nodes;
    std::vector<std::vector<NodeWithInfo *> *> combinations;

    //the below used for sorting, every partition must have all nodes with the same values for the below variables
    int degree;
    std::string node_label;
    std::string nl;

    void combinations_fn(std::vector<NodeWithInfo *> *notused, bool);

public:
    int counter;

    bool operator<(const CL_Partition &str) const;

    void set_sorting_values();

    CL_Partition(int part_id);

    void add_node(NodeWithInfo *n_info);

    std::vector<NodeWithInfo *>::const_iterator get_nodes_enum() const;

    std::vector<NodeWithInfo *>::const_iterator get_nodes_end() const { return nodes.end(); }

    int get_num_nodes();

    void clear_nodes();

    void set_id(int part_id);

    int get_id();

    std::map<std::string, CL_Partition *> *get_new_parts();

    std::string to_string();

    std::vector<std::vector<NodeWithInfo *> *> *get_combinations();

    void sort_nodes();

    void cl_part_destructor();

    friend bool ascending(CL_Partition *a, CL_Partition *b);

    friend bool descending(CL_Partition *a, CL_Partition *b);

    std::string get_prefix();

};

void map_to_vec(std::map<std::string, CL_Partition *> &m, std::vector<CL_Partition *> &v);

bool ascending(CL_Partition *a, CL_Partition *b);

bool descending(CL_Partition *a, CL_Partition *b);

class PartID_label {
public:
    PartID_label() = default;

    ~PartID_label() = default;

    int part_id{};
    std::string label;

    std::string to_string();
};

class CanonicalLabel {
private:
    static const bool enable_print = false;

    static bool sort_partitions(std::vector<CL_Partition *> &parts);

    static char *
    generate_can_label(std::vector<NodeWithInfo *> *nodes, GraphX *graph, std::string **, bool one_line_only = false);

public:
    static char sig_buffer[10000];

    static std::string generate(GraphX *graph);

    static std::string generate_neighbors_list(NodeWithInfo *nInfo, std::map<int, NodeWithInfo *> &allNodes);
};
