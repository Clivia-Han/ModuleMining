/**
 * representing graph edge
 */

#include "MyEdge.h"

MyEdge::MyEdge(std::string label, MyNode *neighbor) {
    this->label = label;
    this->neighbor = neighbor;
}
