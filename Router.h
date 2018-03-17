#ifndef ROUTER_H
#define ROUTER_H

#include <string>
#include <vector>

#define ROUTE_SHP	1
#define ROUTE_SDP	2
#define ROUTE_STP	3
#define ROUTE_FTP	4
#define ROUTE_MGP	5

#define OUTPUT_COL_WIDTH 		8 // 8 is the lowest it can go
#define DOUBLE_STRING_PRECISION 2

#define MAP_SIZE 	26
#define USI unsigned short int

struct Dwarf
{
	std::string name;
	char location;
	
	Dwarf(std::string n, char l):
	name(n),
	location(l)
	{}
};

struct Edge
{
	USI host1;
	USI host2;
	USI distance;
	USI time; // Note: this is in minutes
	USI coins;
	USI trolls;
	bool taken = false;
	
	Edge(USI h1, USI h2, USI dist, USI ti, USI c, USI tr):
	host1(h1),
	host2(h2),
	distance(dist),
	time(ti),
	coins(c),
	trolls(tr)
	{}
};

struct Node
{
	USI locationIndex;
	std::vector<Edge*> edges; // Connecting nodes share the same edge reference! Care when deleting!!! Must be pointers so that traversing through the other host still flags it as used. 
	
	Node(USI loc):
	locationIndex(loc)
	{}
	
	// To avoid deleting an already deleted edge, we only delete edges which have the other host with a lower array index value. (must be lower as if higher is used the check for node location will segment fault)
	// Deleting can work this way as the Node map is deleted in order from 0 to MAP_SIZE
	~Node()
	{
		for(unsigned int i = 0; i < edges.size(); i++)
		{
			// In this case the other host is host2
			if(edges[i]->host1 == locationIndex && edges[i]->host2 < locationIndex)
				delete edges[i];
			
			// In this case the other host is host1
			else if(edges[i]->host2 == locationIndex && edges[i]->host1 < locationIndex)
				delete edges[i];
		}
	}
};

struct NodeValue
{
	std::vector<USI> valuePath; // Note: this needs to be read in reverse as recursion builds it from the end to the start, also the path MUST end in the desired destination node
	unsigned int value = -1;// -1 assigns to max cause unsigned
};

void route(int code);
void routeDij(NodeValue* responseMap); 

// The greed algorithm picks the best response to use from the vector, then adds the edge cost to get to the node holding that vector.
// The case of there being no path should never occur as due to the assumptions listed in the assignment document
NodeValue routeMGP(USI currentNodeIndex, Edge* edgeTaken);
NodeValue greedAlgorithm(Edge* edgeTakenToCurrentNode, std::vector<NodeValue>* responsesForPathsFromCurrentNode, USI currentNodeIndex);

USI followEdgeIndex(Node* nodeOn, Edge* edgeTake);
unsigned int getEdgevalue(Edge* edge);

void printResults(std::vector<NodeValue>* dwarfResponses, std::vector<Dwarf>* dwarfs, std::string dwarfMeetHouseName);
std::string doubleOutput(double d);
void outputFormattedColLeft(std::string value);
void outputFormattedColCentered(std::string value);
void outputFormattedColRight(std::string value);

#endif