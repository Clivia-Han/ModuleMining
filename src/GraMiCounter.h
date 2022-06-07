#ifndef GRAMICOUNTER_H_
#define GRAMICOUNTER_H_

#include <tr1/unordered_set>
#include <vector>
#include "GraphX.h"
#include "Pattern.h"

class pair_with_edge {
public:
    int id1;
    int id2;
    double edge_label;
    int min_domain_size;
};

class GraMiCounter {
private:
    static void
    set_domains_order(tr1::unordered_map<int, tr1::unordered_set<int> *> &domains_values, int *order, Pattern *pattern);

public:
    static int num_by_passed_nodes;
    static bool use_ac3;
    static int num_side_effect_nodes;
    static int num_postponed_nodes;

    static int is_frequent(GraphX *graph, Pattern *pattern, int support, double approximate,
                           map<int, set<int>> &domains_solutions);

    static void AC_3(GraphX *, tr1::unordered_map<int, tr1::unordered_set<int> *> &, Pattern *, int support);

    static void
    AC_3(GraphX *, tr1::unordered_map<int, tr1::unordered_set<int> *> &, GraphX *, int support, int invalid_col = -1);

    static bool refine(GraphX *, tr1::unordered_set<int> *, tr1::unordered_set<int> *, pair_with_edge *, int);

    static bool is_it_acyclic(GraphX &query);

};

#endif /* GRAMICOUNTER_H_ */
