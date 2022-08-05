/**
 * representing graph edge
 */
#pragma once

#include "MyNode.h"

class MyEdge {
private:
    std::string label;
    MyNode *neighbor;   //the neighbor which the edge is linked to

public:
    MyEdge(std::string label, MyNode *neighbor);

    std::string get_label() { return label; }

    MyNode *get_neighbor() { return neighbor; }
};
