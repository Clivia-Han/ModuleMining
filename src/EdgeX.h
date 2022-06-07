/**
 * representing graph edge
 */
#ifndef EDGEX_H_
#define EDGEX_H_

#include "NodeX.h"

class EdgeX {
private:
    double label;    //edge label
    NodeX *other_node;    //edges are originating from one node to the other node, this referes to the other node

public:
    EdgeX(double label, NodeX *other_node);

    double get_label() { return label; }

    NodeX *get_other_node() { return other_node; }
};

#endif /* EDGEX_H_ */
