#ifndef NODEX_H_
#define NODEX_H_

#include <map>
#include<vector>
#include<ostream>
#include <tr1/unordered_map>

using namespace std;

class NodeX
{
private:
	int id;
	double label;
	tr1::unordered_map<int, void*> edges;
	tr1::unordered_map<int, void*> revEdges;

public:
	NodeX(int id, double value);
	~NodeX();
	void addEdge(NodeX* , double edgeLabel, int graphType);
	void removeEdge(NodeX* , int graphType);
	friend ostream& operator<<(ostream& os, const NodeX& n);
	int getID() {return id;}
	double getLabel() {return label;}
	tr1::unordered_map<int, void*>::iterator getEdgesIterator() {return edges.begin();}
	tr1::unordered_map<int, void*>::iterator getEdgesEndIterator() {return edges.end();}
	int getEdgesSize() {return edges.size(); }
	void* getEdgeForDestNode(int destNodeID );
	bool isItConnectedWithNodeID(int nodeID);
	bool isItConnectedWithNodeID(int nodeID, double label);
	bool isNeighborhoodConsistent(NodeX* );
};

#endif /* NODEX_H_ */
