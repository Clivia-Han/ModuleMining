#pragma once
#include "core_file.h"


class Settings {
public:
    static int support;
    static bool debug_msg;
    static int given_flag;
    static int graph_type;
    static std::string path;
    static int max_edges_num;
    static int max_nodes_num;
    static std::string file_name;
    static int given_seed_node_id;
    static bool use_predicted_inv_column;
    static bool throw_nodes_after_iterations;
    static long postpone_nodes_after_iterations;
};
