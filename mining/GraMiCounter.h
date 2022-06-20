#pragma once

#include "GraphX.h"
#include "Pattern.h"
#include "core_file.h"

class pair_with_edge {
public:
    int id1;
    int id2;
    std::string edge_label;
    int min_domain_size;
};

class GraMiCounter {
private:
    static void
    set_domains_order(std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> &domains_values, int *order, Pattern *pattern);

public:
    static int is_frequent(GraphX *graph, Pattern *pattern, int support, double approximate,
                           std::map<int, std::set<int>> &domains_solutions);

    static void AC_3(GraphX *, std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> &, Pattern *, int support);

    static void
    AC_3(GraphX *, std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> &, GraphX *, int support, int invalid_col = -1);

    static bool refine(GraphX *, std::tr1::unordered_set<int> *, std::tr1::unordered_set<int> *, pair_with_edge *, int);

    static bool is_it_acyclic(GraphX &query);

};
