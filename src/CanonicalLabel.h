/**
 * This class is responsible for generating a unique graph canonical label
 */
#ifndef CANONICALLABEL_H_
#define CANONICALLABEL_H_

#include<iostream>
#include<map>
#include<vector>
#include<string>
#include"GraphX.h"

using namespace std;

class NodeWithInfo {
public:
    NodeX *node;
    string nl;//neighbors list
    int part_id;

    NodeWithInfo(NodeX *node);

    ~NodeWithInfo() {
        NodeWithInfo::st_counter--;
    }

    string to_string();

    bool operator<(const NodeWithInfo &) const;

    friend bool NodeWithInfo_ascending(NodeWithInfo *a, NodeWithInfo *b);

    friend bool NodeWithInfo_descending(NodeWithInfo *a, NodeWithInfo *b);

    static int st_counter;
};

bool NodeWithInfo_ascending(NodeWithInfo *a, NodeWithInfo *b);

bool NodeWithInfo_descending(NodeWithInfo *a, NodeWithInfo *b);

class CL_Partition {
private:
    int part_id;
    vector<NodeWithInfo *> nodes;
    vector<vector<NodeWithInfo *> *> combinations;

    //the below used for sorting, every partition must have all nodes with the same values for the below variables
    int degree;
    string node_label;
    string nl;

    void combinations_fn(vector<NodeWithInfo *> *notused, bool);

public:
    static int st_counter;
    int counter;

    bool operator<(const CL_Partition &str) const;

    void set_sorting_values();

    CL_Partition(int part_id);

    void add_node(NodeWithInfo *n_info);

    vector<NodeWithInfo *>::const_iterator get_nodes_enum() const;

    vector<NodeWithInfo *>::const_iterator get_nodes_end() const { return nodes.end(); }

    int get_num_nodes();

    void clear_nodes();

    void set_id(int part_id);

    int get_id();

    map<string, CL_Partition *> *get_new_parts();

    string to_string();

    vector<vector<NodeWithInfo *> *> *get_combinations();

    void sort_nodes();

    void cl_part_destructor();

    friend bool ascending(CL_Partition *a, CL_Partition *b);

    friend bool descending(CL_Partition *a, CL_Partition *b);

    string get_prefix();
};

bool ascending(CL_Partition *a, CL_Partition *b);

bool descending(CL_Partition *a, CL_Partition *b);

class PartID_label {
public:
    int static st_counter;

    PartID_label() { PartID_label::st_counter++; }

    ~PartID_label() { PartID_label::st_counter--; }

    int part_id;
    string label;

    string to_string();
};

class CanonicalLabel {
private:
    static const bool enable_print = false;

    static bool sort_partitions(vector<CL_Partition *> &parts);

    static char *
    generate_can_label(vector<NodeWithInfo *> *nodes, GraphX *graph, double **, bool one_line_only = false);

public:
    static char sig_buffer[10000];

    static string generate(GraphX *graph);

    static string generate_neighbors_list(NodeWithInfo *nInfo, map<int, NodeWithInfo *> &allNodes);
};

#endif /* CANONICALLABEL_H_ */
