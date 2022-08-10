#pragma once

#include "MyGraph.h"
#include "core_file.h"

class NodeInfo {
public:
    MyNode *node;
    std::string nl;//neighbors list
    int part_id;

    NodeInfo(MyNode *node);

    ~NodeInfo() = default;

    std::string to_string();

    bool operator<(const NodeInfo &) const;

    friend bool node_info_asc(NodeInfo *a, NodeInfo *b);

    friend bool node_info_des(NodeInfo *a, NodeInfo *b);

};

bool node_info_asc(NodeInfo *a, NodeInfo *b);

bool node_info_des(NodeInfo *a, NodeInfo *b);

class Sig_Partition {
private:
    int part_id;
    std::vector<NodeInfo *> nodes;
    std::vector<std::vector<NodeInfo *> *> combinations;

    int degree;
    std::string node_label;
    std::string nl;

    void combinations_fn(std::vector<NodeInfo *> *notused, bool);

public:
    int counter;

    bool operator<(const Sig_Partition &str) const;

    void set_sorting_values();

    Sig_Partition(int part_id);

    void add_node(NodeInfo *n_info);

    std::vector<NodeInfo *>::const_iterator get_nodes_enum() const;

    std::vector<NodeInfo *>::const_iterator get_nodes_end() const { return nodes.end(); }

    int get_num_nodes();

    void clear_nodes();

    void set_id(int part_id);

    int get_id();

    std::map<std::string, Sig_Partition *> *get_new_parts();

    std::string to_string();

    std::vector<std::vector<NodeInfo *> *> *get_combinations();

    void sort_nodes();

    void sig_part_destructor();

    friend bool ascending(Sig_Partition *a, Sig_Partition *b);

    friend bool descending(Sig_Partition *a, Sig_Partition *b);

    std::string get_prefix();

};

void map_to_vec(std::map<std::string, Sig_Partition *> &m, std::vector<Sig_Partition *> &v);

bool ascending(Sig_Partition *a, Sig_Partition *b);

bool descending(Sig_Partition *a, Sig_Partition *b);

class PartID_label {
public:
    PartID_label() = default;

    PartID_label(int _part_id, std::string _label) : part_id(_part_id), label(_label) {}

    ~PartID_label() = default;

    int part_id{};
    std::string label;

    std::string to_string();
};

class Signature {
private:
    static const bool enable_print = false;

    static bool sort_partitions(std::vector<Sig_Partition *> &parts);

    static char *
    generate_can_label(std::vector<NodeInfo *> *nodes, MyGraph *graph, std::string **, bool one_line_only = false);

public:
    static char sig_buffer[10000];

    static std::string generate(MyGraph *graph);

    static std::string generate_neighbors_list(NodeInfo *nInfo, std::map<int, NodeInfo *> &allNodes);
};
