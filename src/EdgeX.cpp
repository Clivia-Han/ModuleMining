/**
 * representing graph edge
 */

#include "EdgeX.h"

EdgeX::EdgeX(double label, NodeX *other_node) {
    this->label = label;
    this->other_node = other_node;
}
