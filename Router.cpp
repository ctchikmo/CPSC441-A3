#include <iostream>
#include <sys/stat.h> // For file existance checking
#include <unistd.h> // Unix standard lib
#include <fstream>
#include <cstring>

#include "Router.h"

#define USI unsigned short int

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
			
			unsigned char host1Index = host1[0] - 'A';
			if(map[host1Index] == NULL)
				map[host1Index] = new Node(host1[0]);
			
			unsigned char host2Index = host2[0] - 'A';
			if(map[host2Index] == NULL)
				map[host2Index] = new Node(host2[0]);
			
			try
			{
				map[host1Index]->edges.push_back(Edge(host2[0], (USI)std::stoi(std::string(dist)), (USI)std::stoi(std::string(time)), (USI)std::stoi(std::string(coins)), (USI)std::stoi(std::string(trolls))));
				map[host2Index]->edges.push_back(Edge(host1[0], (USI)std::stoi(std::string(dist)), (USI)std::stoi(std::string(time)), (USI)std::stoi(std::string(coins)), (USI)std::stoi(std::string(trolls))));
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
	std::cout << "Routing everyone to " << dwarfMeetHouseName << "'s house" << std::endl;
	
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
	
	// Delete dynamic memory
	for(unsigned int i = 0; i < MAP_SIZE; i++)
		delete map[i];
}

void dispayMessage(DwarfRouteMessage* drm)
{
	
}

void routeAll(Node** map, char meetNode, Dwarf* dwarf)
{
	
}

void routeSHP(Node** map, char meetNode, Dwarf* dwarf)
{
	
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



