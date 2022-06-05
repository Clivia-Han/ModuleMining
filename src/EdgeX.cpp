/**
 * representing graph edge
 */

#include "EdgeX.h"

EdgeX::EdgeX(double label, NodeX* otherNode)
{
	this->label = label;
	this->otherNode = otherNode;
}
