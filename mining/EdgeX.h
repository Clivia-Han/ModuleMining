/**
 * representing graph edge
 */
#pragma once

#include "NodeX.h"

class EdgeX {
private:
    std::string label;    //edge label
    NodeX *other_node;    //edges are originating from one node to the other node, this referes to the other node

public:
    EdgeX(std::string label, NodeX *other_node);

    std::string get_label() { return label; }

    NodeX *get_other_node() { return other_node; }
};
