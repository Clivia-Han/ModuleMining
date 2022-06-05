/**
 * Represent a graph node
 */

#include<iostream>
#include <stdlib.h>
#include<tr1/unordered_map>
#include "EdgeX.h"
#include "NodeX.h"

NodeX::NodeX(int id, double label)
{
	this->id = id;
	this->label = label;
}

NodeX::~NodeX()
{
	for( tr1::unordered_map<int, void*>::const_iterator ii=edges.begin(); ii!=edges.end(); ++ii)
    {
    	EdgeX* edge = (EdgeX*)((*ii).second);
    	delete edge;
    }
	edges.clear();

	for( tr1::unordered_map<int, void*>::const_iterator ii=revEdges.begin(); ii!=revEdges.end(); ++ii)
	{
	  	EdgeX* edge = (EdgeX*)((*ii).second);
	   	delete edge;
	}
	revEdges.clear();
}

/**
 * Add an edge to this node
 * Parameters: the other node, and the edge label
 */
void NodeX::addEdge(NodeX* otherNode, double edgeLabel, int graphType)
{
	// edges为当前结点关联的所有边
	if(edges.find(otherNode->getID())!=edges.end())
		return;
	EdgeX* edge = new EdgeX(edgeLabel, otherNode);

	edges[otherNode->getID()] = edge;

	if(graphType==1)
	{
		EdgeX* edge = new EdgeX(edgeLabel, this);
		otherNode->revEdges[this->getID()] = edge;
	}
}

void NodeX::removeEdge(NodeX* otherNode, int graphType)
{
	tr1::unordered_map<int, void*>::iterator iter = edges.find(otherNode->getID());
	if(iter!=edges.end())
		delete iter->second;
	edges.erase(otherNode->getID());

	if(graphType==1)
	{
		//do somethingh here
		tr1::unordered_map<int, void*>::iterator iter = otherNode->revEdges.find(this->getID());
		if(iter!=revEdges.end())
			delete iter->second;
		revEdges.erase(this->getID());
	}
}

void* NodeX::getEdgeForDestNode(int destNodeID )
{
	tr1::unordered_map<int, void*>::iterator temp = edges.find(destNodeID);
	if(temp==edges.end())
		return NULL;
	else
		return (*temp).second;
}

bool NodeX::isItConnectedWithNodeID(int nodeID)
{
	//check node connectivity
	tr1::unordered_map<int, void*>::iterator iter = edges.find(nodeID);
	if(iter==edges.end())
		return false;

	return true;
}

bool NodeX::isItConnectedWithNodeID(int nodeID, double label)
{
	//check node connectivity
	tr1::unordered_map<int, void*>::iterator iter = edges.find(nodeID);
	if(iter==edges.end())
		return false;

	//check edge label
	if(((EdgeX*)iter->second)->getLabel()!=label)
		return false;

	return true;
}

/**
 * check whether the 'node' parameter in neighborhood consistent with 'this' node
 */
bool NodeX::isNeighborhoodConsistent(NodeX* node)
{
	//该方法实际上是在比较当前节点与node节点的所有邻居中，每个标签出现的总次数（标签分布）是否相等
	tr1::unordered_map<double, int> labels;
	//populate labels of this node
	for(tr1::unordered_map<int, void*>::iterator iter = edges.begin();iter!=edges.end();iter++)
	{
		//统计当前节点的相邻节点的标签，计算每个标签对应的出现次数
		double otherNodeLabel = ((EdgeX*)iter->second)->getLabel();
		tr1::unordered_map<double, int>::iterator tempIter = labels.find(otherNodeLabel);
		if(tempIter==labels.end())
			labels.insert(std::pair<double, int>(otherNodeLabel, 1));
		else
		{
			int currentCount = tempIter->second;
			labels.erase(otherNodeLabel);
			labels.insert(std::pair<double, int>(otherNodeLabel, currentCount+1));
		}
	}

	//check labels against this's labels
	for(tr1::unordered_map<int, void*>::iterator iter = node->getEdgesIterator();iter!=node->getEdgesEndIterator();iter++)
	{
		//遍历node节点的相邻节点
		double otherNodeLabel = ((EdgeX*)iter->second)->getLabel();
		//如果node节点的相邻节点对应的标签在当前节点的相邻节点标签集中未出现，返回false
		tr1::unordered_map<double, int>::iterator tempIter = labels.find(otherNodeLabel);
		if(tempIter==labels.end())
			return false;
		//否则将该标签在labels字典中的对应值-1
		int currentCount = tempIter->second;
		labels.erase(otherNodeLabel);
		if(currentCount>1)	//如果currentCount<1，那么相当于从labels字典中直接去掉otherNodeLabel
			labels.insert(std::pair<double, int>(otherNodeLabel, currentCount-1));
	}

	return true;
}

ostream& operator<<(ostream& os, const NodeX& n)
{
	//用指定格式输出节点n及其项链的所有边的信息（包括边标签和连接点）到os
    os << n.id << '[' << n.label << "]" <<endl;
    for( tr1::unordered_map<int, void*>::const_iterator ii=n.edges.begin(); ii!=n.edges.end(); ++ii)
    {
    	EdgeX* edge = (EdgeX*)((*ii).second);
    	os<<"--"<<edge->getLabel()<<"-->"<<edge->getOtherNode()->getID()<<endl;
    }
    return os;
}
