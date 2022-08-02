#pragma once

#include "MyGraph.h"
#include "Pattern.h"
#include "core_file.h"

class EdgeInfo {
public:
    int id1;
    int id2;
    std::string edge_label;
    int min_domain_size;
};

class CSPSolver {
public:
    static bool check_acyclic(MyGraph &query);

    static bool refine(MyGraph *, std::tr1::unordered_set<int> *, std::tr1::unordered_set<int> *, EdgeInfo *, int);

    static void
    check_ac(MyGraph *, std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> &, Pattern *, int support);

    static void
    check_ac(MyGraph *, std::tr1::unordered_map<int, std::tr1::unordered_set<int> *> &, MyGraph *, int support,
             int invalid_col = -1);

    static int get_frequency(MyGraph *graph, Pattern *pattern, int support, double approximate,
                             std::map<int, std::set<int>> &domains_solutions);
};
