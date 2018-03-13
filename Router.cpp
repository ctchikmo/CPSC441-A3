#include <iostream>
#include <sys/stat.h> // For file existance checking
#include <unistd.h> // Unix standard lib
#include <fstream>
#include <cstring>

#include "Router.h"

// Thanks @https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c for file existance checking
void route(int code)
{
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
	
	Node* map[26];
	for(int i = 0; i < 26; i++) 
		map[i] = NULL;
	
	// Read in the topology file
	std::ifstream topologyFile(topologyFileName); 
	if(topologyFile)
	{
		std::string line;
		while(std::getline(topologyFile, line)) // Read in whole line, we expect 6 inputs on each line
		{
			// To understand how this whole section works look at: http://www.cplusplus.com/reference/cstring/strtok/ I used this instead of (ifstream >>) so that I could ignore \r, and could check lines. 
			char* loc = std::strtok(line.c_str(), " \r\n");
			char* dest = std::strtok(NULL, " \r\n");
			char* dist = std::strtok(NULL, " \r\n");
			char* time = std::strtok(NULL, " \r\n");
			char* coins = std::strtok(NULL, " \r\n");
			char* trolls = std::strtok(NULL, " \r\n");
			
			if(loc == NULL || loc[0] < 'A' || loc[0] > 'Z' || dest == NULL || dest[0] < 'A' || dest[0] > 'Z' || dist == NULL || time == NULL || coins == NULL || trolls == NULL)
			{
				std::cout << "Error, line in topology file has invalid value." << std::endl << std::endl;
				return;
			}
			
			unsigned char index = loc[0] - 'A';
			if(map[index] == NULL)
				map[index] = new Node(loc[0]);
			
			map[index]->edges.push_back(Edge(dest[0], (unsigned short int)std::stoi(std::string(dist)), (unsigned short int)std::stoi(std::string(time)),
											(unsigned short int)std::stoi(std::string(coins)), (unsigned short int)std::stoi(std::string(trolls))));
		}
		
		topologyFile.close();
	}
	else
	{
		std::cout << "Error opening topology file." << std::endl << std::endl;
		return;
	}
	
	
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
	
	// Read in the dwarf file, ensure the house nodes all exist
}

void dispayMessage(DwarfRouteMessage* drm)
{
	
}

void routeAll(Node** map, Dwarf* dwarf)
{
	
}

void routeSHP(Node** map, Dwarf* dwarf)
{
	
}

void routeSDP(Node** map, Dwarf* dwarf)
{
	
}

void routeSTP(Node** map, Dwarf* dwarf)
{
	
}

void routeFTP(Node** map, Dwarf* dwarf)
{
	
}

void routeMGP(Node** map, Dwarf* dwarf)
{
	
}



