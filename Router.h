#ifndef ROUTER_H
#define ROUTER_H

#include <string>
#include <vector>

#define ROUTE_ALL 	0
#define ROUTE_SHP	1
#define ROUTE_SDP	2
#define ROUTE_STP	3
#define ROUTE_FTP	4
#define ROUTE_MGP	5

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

struct DwarfRouteMessage
{
	Dwarf* dwarf;
	std::vector<char> hops; // This vector does not include the starting location, but does include the destination. Number of hops is the size of this
	unsigned int totalDistance; // Total distance might exceed 16 bits, so this is not a short int
	unsigned int totalTime;
	USI totalCoins;
	USI totalTrolls;
};

struct GreedResponse
{
	std::vector<USI> greedPath; // Note: this needs to be read in reverse as recursion builds it from the end to the start, also the path MUST end in the desired destination node
	unsigned int greedValue;
};

void route(int code);

// The greed algorithm picks the best response to use from the vector, then adds the edge cost to get to the node holding that vector.
// The case of there being no path should never occur as due to the assumptions listed in the assignment document
GreedResponse greedAlgorithm(Edge* edgeTakenToCurrentNode, std::vector<GreedResponse>* responsesForPathsFromCurrentNode, USI currentNodeIndex, USI destIndex, USI algCode);

// We return the greedy sum of what the best path for the given greedy function is from the node (determined by aglCode), plus the cost to get to the node.
GreedResponse recursivePathFinder(Node** map, USI meetNodeIndex, USI currentNodeIndex, Edge* edgeTaken, USI algCode, bool stopAtDest); 
int followEdgeIndex(Node* nodeOn, Edge* edgeTake);

// Note: for both of these ROUTE_ALL is not a valid code (no checks, just don't use it!)
unsigned int getEdgeGreedValue(Edge* edge, USI algCode);
bool compareGreedValues(unsigned int pathToCheckGV, unsigned int bestGV, USI algCode);

#endif