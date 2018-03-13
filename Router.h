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
	char destination;
	unsigned short int distance;
	unsigned short int time; // Note: this is in minutes
	unsigned short int coins;
	unsigned short int trolls;
	
	Edge(char dest, unsigned short int dist, unsigned short int ti, unsigned short int c, unsigned short int tr):
	destination(dest),
	distance(dist),
	time(ti),
	coins(c),
	trolls(tr)
	{}
};

struct Node
{
	char location;
	std::vector<Edge> edges;
	
	Node(char loc):
	location(loc)
	{}
};

struct DwarfRouteMessage
{
	Dwarf* dwarf;
	std::vector<char> hops; // This vector does not include the starting location, but does include the destination. Number of hops is the size of this
	unsigned int totalDistance; // Total distance might exceed 16 bits, so this is not a short int
	unsigned int totalTime;
	unsigned short int totalCoins;
	unsigned short int totalTrolls;
};

void route(int code);
void dispayMessage(DwarfRouteMessage* drm);

// Node*** is an array[26] as we have a max of 26 nodes*. This is implemented as a Hashmap, with the key being (loc - 'A'). Not a vector cause mapping the edges becomes expontentially long to do. 
void routeAll(Node** map, Dwarf* dwarf);
void routeSHP(Node** map, Dwarf* dwarf);
void routeSDP(Node** map, Dwarf* dwarf);
void routeSTP(Node** map, Dwarf* dwarf);
void routeFTP(Node** map, Dwarf* dwarf);
void routeMGP(Node** map, Dwarf* dwarf);

#endif