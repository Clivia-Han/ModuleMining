#ifndef SETTINGS_H_
#define SETTINGS_H_


class Settings {
public:
    static int max_number_of_samples;
    static int min_number_of_samples;
    static long postponeNodesAfterIterations;
    static bool divide_big_tasks;
    constexpr static double valids_per_task_margin = 0.8;
    static bool debug_msg;
    static long postpone_nodes_after_iterations;
    static int max_subgraph_size;
    static int max_num_nodes;
    static bool use_predicted_inv_column;

    static int fixed_num_subtasks;
    static double min_imbalance;
    static int given_type;
};

#endif /* SETTINGS_H_ */
