/**
 * representing graph edge
 */
#ifndef EDGEX_H_
#define EDGEX_H_

#include "NodeX.h"

class EdgeX
{
private:
	double label;	//edge label
	NodeX* otherNode;	//edges are originating from one node to the other node, this referes to the other node

public:
	EdgeX(double label, NodeX* otherNode);
	double getLabel() {return label;}
	NodeX* getOtherNode(){return otherNode;}
};

#endif /* EDGEX_H_ */
