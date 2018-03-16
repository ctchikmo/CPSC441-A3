#include <iostream>
#include <sys/stat.h> // For file existance checking
#include <unistd.h> // Unix standard lib
#include <fstream>
#include <cstring>
#include <climits> // MAX values for data types

#include "Router.h"

// Thanks @https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c for file existance checking
void route(int code)
{
	std::cout << "Please enter the name or path to the topology file" << std::endl;
	
	std::string topologyFileName;
	while(std::getline(std::cin, topologyFileName))
	{
		if(topologyFileName.size() == 1 && topologyFileName[0] == 'q')
		{
			std::cout << "Stopped while entering topology file. " << std::endl << std::endl;
			return;
		}
		
		struct stat buffer;   
		if(stat(topologyFileName.c_str(), &buffer) == 0) // If == 0 than no error, thus file exists, so break
			break;
		else
			std::cout << "Could not find file, try again or enter 'q' to stop" << std::endl;
	}
	std::cout << "Topology file found" << std::endl << std::endl;
	
	Node* map[MAP_SIZE];
	for(int i = 0; i < MAP_SIZE; i++) 
		map[i] = NULL;
	
	// Read in the topology file
	std::ifstream topologyFile(topologyFileName); 
	if(topologyFile)
	{
		std::string line;
		while(std::getline(topologyFile, line)) // Read in whole line, we expect 6 inputs on each line
		{
			// To understand how this whole section works look at: http://www.cplusplus.com/reference/cstring/strtok/ I used this instead of (ifstream >>) so that I could ignore \r, and could check lines. 
			char temp[line.size() + 1];
			for(unsigned int i = 0; i < line.size(); i++)
				temp[i] = line[i];
			temp[line.size()] = '\0';
			
			char* host1 = std::strtok(temp, " \r\n");
			char* host2 = std::strtok(NULL, " \r\n");
			char* dist = std::strtok(NULL, " \r\n");
			char* time = std::strtok(NULL, " \r\n");
			char* coins = std::strtok(NULL, " \r\n");
			char* trolls = std::strtok(NULL, " \r\n");
			
			if(host1 == NULL || host1[0] < 'A' || host1[0] > 'Z' || host2 == NULL || host2[0] < 'A' || host2[0] > 'Z' || dist == NULL || time == NULL || coins == NULL || trolls == NULL)
			{
				std::cout << "Error, line in topology file has invalid value." << std::endl << std::endl;
				return;
			}
			
			USI host1Index = host1[0] - 'A';
			if(map[host1Index] == NULL)
				map[host1Index] = new Node(host1Index);
			
			USI host2Index = host2[0] - 'A';
			if(map[host2Index] == NULL)
				map[host2Index] = new Node(host2Index);
			
			try
			{
				Edge* edge = new Edge(host1Index, host2Index, (USI)std::stoi(std::string(dist)), (USI)std::stoi(std::string(time)), (USI)std::stoi(std::string(coins)), (USI)std::stoi(std::string(trolls)));
				map[host1Index]->edges.push_back(edge);
				map[host2Index]->edges.push_back(edge);
			}
			catch(...)
			{
				std::cout << "Error, line in topology file has invalid value." << std::endl << std::endl;
				return;
			}
		}
		
		topologyFile.close();
	}
	else
	{
		std::cout << "Error opening topology file." << std::endl << std::endl;
		return;
	}
	
	std::cout << "Please enter the name or path to the dwarf file" << std::endl;
	
	std::string dwarfFileName;
	while(std::getline(std::cin, dwarfFileName))
	{
		if(dwarfFileName.size() == 1 && dwarfFileName[0] == 'q')
		{
			std::cout << "Stopped while entering dwarf file. " << std::endl << std::endl;
			return;
		}
		
		struct stat buffer;   
		if(stat(dwarfFileName.c_str(), &buffer) == 0) // If == 0 than no error, thus file exists, so break
			break;
		else
			std::cout << "Could not find file, try again or enter 'q' to stop" << std::endl;
	}
	std::cout << "Dwarf file found" << std::endl << std::endl;
	
	std::vector<Dwarf> dwarfs;
	
	// Read in the dwarf file, ensure the house nodes all exist
	std::ifstream dwarfFile(dwarfFileName); 
	if(dwarfFile)
	{
		std::string line;
		while(std::getline(dwarfFile, line)) // Read in whole line, we expect 6 inputs on each line
		{
			// To understand how this whole section works look at: http://www.cplusplus.com/reference/cstring/strtok/ I used this instead of (ifstream >>) so that I could ignore \r, and could check lines. 
			char temp[line.size() + 1];
			for(unsigned int i = 0; i < line.size(); i++)
				temp[i] = line[i];
			temp[line.size()] = '\0';
			
			char* name = std::strtok(temp, " \r\n");
			char* loc = std::strtok(NULL, " \r\n");
			
			if(name == NULL || loc == NULL || loc[0] < 'A' || loc[0] > 'Z')
			{
				std::cout << "Error, line in dwarf file has invalid value." << std::endl << std::endl;
				return;
			}
			
			unsigned char index = loc[0] - 'A';
			if(map[index] == NULL)
			{
				std::cout << "Error, line in dwarf file has invalid home." << std::endl << std::endl;
				return;
			}
			
			dwarfs.push_back(Dwarf(std::string(name), loc[0]));
		}
		
		dwarfFile.close();
	}
	else
	{
		std::cout << "Error opening dwarf file." << std::endl << std::endl;
		return;
	}
	
	// Get the dwarfs house to meet at, default to Bilbo
	std::string dwarfMeetHouseName;
	
	std::cout << "Please enter the name of the dwarf whose house everyone should meet at. (Defaults to Bilbo if the name is invalid)" << std::endl;
	std::getline(std::cin, dwarfMeetHouseName);
	
	Label_getDwarfMeetLocation: // NOTICE THIS LABEL, IT CONNECTS TO THE GOTO IN THE SINGLE IF AFTER THIS FOR LOOP
	char dwarfMeetNode = '\0';
	for(unsigned int i = 0; i < dwarfs.size(); i++)
		if(dwarfs[i].name.compare(dwarfMeetHouseName) == 0)
			dwarfMeetNode = dwarfs[i].location;
		
	if(dwarfMeetNode == '\0')
	{
		std::cout << "Could not find the dwarf with the name: " << dwarfMeetHouseName << std::endl << std::endl;
		if(dwarfMeetHouseName.compare("Bilbo") != 0) // Bilbo was not entered, check again using Bilbo (NOTICE THE GOTO!!!!!)
		{
			std::cout << "Name entered was not found, and did not exactly match \"Bilbo\". Defaulting to Bilbo" << std::endl;
			dwarfMeetHouseName.assign("Bilbo");
			goto Label_getDwarfMeetLocation;
		}			
		return;
	}
	std::cout << "Routing everyone to " << dwarfMeetHouseName << "'s house" << std::endl << std::endl;
	
	// Call the appropriate route method.
	switch(code)
	{
		case ROUTE_ALL:
			for(unsigned int i = 0; i < dwarfs.size(); i++) 
				routeAll(map, dwarfMeetNode, &dwarfs[i]);
			break;
			
		case ROUTE_SHP:
			for(unsigned int i = 0; i < dwarfs.size(); i++) 
				routeSHP(map, dwarfMeetNode, &dwarfs[i]);
			break;
			
		case ROUTE_SDP:
			for(unsigned int i = 0; i < dwarfs.size(); i++) 
				routeSDP(map, dwarfMeetNode, &dwarfs[i]);
			break;
			
		case ROUTE_STP:
			for(unsigned int i = 0; i < dwarfs.size(); i++) 
				routeSTP(map, dwarfMeetNode, &dwarfs[i]);
			break;
			
		case ROUTE_FTP:
			for(unsigned int i = 0; i < dwarfs.size(); i++) 
				routeFTP(map, dwarfMeetNode, &dwarfs[i]);
			break;
			
		case ROUTE_MGP:
			for(unsigned int i = 0; i < dwarfs.size(); i++) 
				routeMGP(map, dwarfMeetNode, &dwarfs[i]);
			break;
	}
	
	// Delete dynamic memory, deletion of edges is handled by the node. 
	for(unsigned int i = 0; i < MAP_SIZE; i++)
		delete map[i];
}

void dispayMessage(DwarfRouteMessage* drm)
{
	
}

void routeAll(Node** map, char meetNode, Dwarf* dwarf)
{
	routeSHP(map, meetNode, dwarf);
	routeSDP(map, meetNode, dwarf);
	routeSTP(map, meetNode, dwarf);
	routeFTP(map, meetNode, dwarf);
	routeMGP(map, meetNode, dwarf);
}

void routeSHP(Node** map, char meetNode, Dwarf* dwarf)
{
	GreedResponse rv = recursivePathFinder(map, (USI)(meetNode - 'A'), (USI)(dwarf->location - 'A'), NULL, greedSHP, true);
	
	// We started at the node
	if(rv.greedPath.size() == 0)
		std::cout << dwarf->name << "path: started at the destination.";
	
	std::cout << dwarf->name << " path: ";
	for(int i = rv.greedPath.size() - 1; i >= 0; i--) // Not unsigned, cause if size is 0 than we fault ( 0 - 1 == USINT MAX)
		std::cout << (char)(rv.greedPath[i] + 'A') << ' ';
	std::cout << std::endl;
}

GreedResponse greedSHP(Edge* edgeTakenToCurrentNode, std::vector<GreedResponse>* responsesForPathsFromCurrentNode, USI currentNodeIndex, USI destIndex)
{
	if(responsesForPathsFromCurrentNode->size() > 0)
	{
		GreedResponse* bestResponseFromVec = NULL;
		unsigned int i = 0;
		
		// This loop picks the initial response to compare to, must be done like this not picking [0] so that we don't pick a response that doesnt end in the dest accidentily. 
		for(; i < responsesForPathsFromCurrentNode->size() && bestResponseFromVec == NULL; i++)
			if((((*responsesForPathsFromCurrentNode)[i]).greedPath[0] == destIndex))
				bestResponseFromVec = &((*responsesForPathsFromCurrentNode)[i]);
		
		// Pick the best response from the remaining depending on the greed factor
		// MUST END AT DEST, greed method is called end backwards, so this is always possible (also the dest is first index as this is done in reverse!!!)
		for(; i < responsesForPathsFromCurrentNode->size(); i++)
			if(((*responsesForPathsFromCurrentNode)[i].greedValue < bestResponseFromVec->greedValue) && (((*responsesForPathsFromCurrentNode)[i]).greedPath[0] == destIndex))
				bestResponseFromVec = &((*responsesForPathsFromCurrentNode)[i]);
			
		if(bestResponseFromVec != NULL)
		{
			// bestResponseFromVec already has the previous nodes best values, so now we just add our edge values
			bestResponseFromVec->greedValue += 1; // 1 Hop
			bestResponseFromVec->greedPath.push_back(currentNodeIndex);
			return *bestResponseFromVec;
		}
	}
	
	// This case happens at the last node. 
	GreedResponse rv;
	rv.greedValue = 1; // 1 Hop to get to this edge
	rv.greedPath.push_back(currentNodeIndex);
	return rv;
}

void routeSDP(Node** map, char meetNode, Dwarf* dwarf)
{
	
}

void routeSTP(Node** map, char meetNode, Dwarf* dwarf)
{
	
}

void routeFTP(Node** map, char meetNode, Dwarf* dwarf)
{
	
}

void routeMGP(Node** map, char meetNode, Dwarf* dwarf)
{
	
}

GreedResponse recursivePathFinder(Node** map, USI meetNodeIndex, USI currentNodeIndex, Edge* edgeTaken, GreedResponse (*greedFunction)(Edge*, std::vector<GreedResponse>*, USI, USI), bool stopAtDest)
{	
	std::vector<GreedResponse> greedResponses; // Each unique next node reachable from this path will insert a response here. We pick which to use via the greed function
	if(stopAtDest && map[currentNodeIndex]->locationIndex == meetNodeIndex)
	{
		// The if below is not included in the if above as I don't want to go into the else in the case that the edgeTaken != NULL
		if(edgeTaken == NULL) // Edge taken is only null if its the first call (so not a recursive call). This means no path is needed, we started on the node destination.
		{
			GreedResponse startAtDestResp; // NOTE: if this case happens, the returned array size for the path will be 0. Therefore we went no where.
			return startAtDestResp;
		}
	}
	else
	{
		for(unsigned int i = 0; i < map[currentNodeIndex]->edges.size(); i++)
		{
			// Take the edge if it is not taken
			if(!map[currentNodeIndex]->edges[i]->taken)
			{
				map[currentNodeIndex]->edges[i]->taken = true;
				greedResponses.push_back(recursivePathFinder(map, meetNodeIndex, followEdgeIndex(map[currentNodeIndex], map[currentNodeIndex]->edges[i]), map[currentNodeIndex]->edges[i], greedFunction, stopAtDest));
			
				// We are back from all of the possible ways taking the above edge can go, so unmark it as taken so the next edge from this node can path back through it, if it is a possible path.
				map[currentNodeIndex]->edges[i]->taken = false;
			}
		}
	}
	
	return greedFunction(edgeTaken, &greedResponses, currentNodeIndex, meetNodeIndex);
}

int followEdgeIndex(Node* nodeOn, Edge* edgeTake)
{
	if(nodeOn->locationIndex == edgeTake->host1)
		return edgeTake->host2;
	
	return edgeTake->host1;
}










