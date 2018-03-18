#include <iostream>
#include <sys/stat.h> // For file existance checking
#include <unistd.h> // Unix standard lib
#include <fstream>
#include <cstring>
#include <climits> // MAX values for data types
#include <list>
#include <iomanip> // setprecision
#include <sstream> // stringstream
#include <pthread.h>

#include "Router.h"
#include "MGPThread.h"

short int threadCount;
pthread_mutex_t listMutex;
pthread_cond_t stackCond;
std::list<MGPThread*> threadList;

// Some global values set everytime a request is entered. 
USI totalIndividuals = 0;
USI destIndex = 0;
USI algCode = 0;
bool stopAtDest = true;
Node* map[MAP_SIZE];

void setupMGPThreads(short int num)
{
	for(int i = 0; i < MAP_SIZE; i++) 
		map[i] = NULL;
	
	threadCount = num;
	pthread_mutex_init(&listMutex, NULL);
	pthread_cond_init(&stackCond, NULL);
	
	for(int i = 0; i < num; i++)
		new MGPThread(); // They will add themselves to threadList
}

void mgpFinished(MGPThread* thread)
{
	pthread_mutex_lock(&listMutex);
	{
		threadList.push_back(thread);
		pthread_cond_signal(&stackCond);
	}
	pthread_mutex_unlock(&listMutex);
}

// Thanks @https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c for file existance checking
void route(int code)
{
	// Setting globals
	algCode = code;
	stopAtDest = (code != ROUTE_MGP);
	
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
	
	// Read in the topology file, note we must update the mgp threads edgeTaken matrix for each new file as the size could be different (during this time the threads in all in wait mode). 
	std::ifstream topologyFile(topologyFileName); 
	Node* newMap[MAP_SIZE];
	for(int i = 0; i < MAP_SIZE; i++) 
		newMap[i] = NULL;
	
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
			if(newMap[host1Index] == NULL)
				newMap[host1Index] = new Node(host1Index);
			
			USI host2Index = host2[0] - 'A';
			if(newMap[host2Index] == NULL)
				newMap[host2Index] = new Node(host2Index);
			
			try
			{
				Edge* edge = new Edge(host1Index, host2Index, (USI)std::stoi(std::string(dist)), (USI)std::stoi(std::string(time)), (USI)std::stoi(std::string(coins)), (USI)std::stoi(std::string(trolls)));
				newMap[host1Index]->edges.push_back(edge);
				newMap[host2Index]->edges.push_back(edge);
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
	
	// Set the mgp threads takenMatrix
	if(code == ROUTE_MGP)
		for(std::list<MGPThread*>::iterator it = threadList.begin(); it != threadList.end(); it++)
			(*it)->buildTakenMatrix(map, newMap);
	
	// Delete dynamic memory, deletion of edges is handled by the node. See note at the end of the function as to why this is not at the end. 
	for(unsigned int i = 0; i < MAP_SIZE; i++)
		if(map[i] != NULL)
			delete map[i];
	
	for(unsigned int i = 0; i < MAP_SIZE; i++)
		map[i] = newMap[i];
	std::cout << "Please enter the name or path to the starting locations file" << std::endl;
	
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
	std::cout << "Starting locations file found" << std::endl << std::endl;
	
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
	totalIndividuals = dwarfs.size();
	
	std::cout << "Please enter the indivudals name that everyone will go to. (Defaults to Bilbo if the name is invalid)" << std::endl;
	std::getline(std::cin, dwarfMeetHouseName);
	
	Label_getDwarfMeetLocation: // NOTICE THIS LABEL, IT CONNECTS TO THE GOTO IN THE SINGLE IF AFTER THIS FOR LOOP
	char dwarfMeetNode = '\0';
	for(unsigned int i = 0; i < dwarfs.size(); i++)
		if(dwarfs[i].name.compare(dwarfMeetHouseName) == 0)
			dwarfMeetNode = dwarfs[i].location;
		
	if(dwarfMeetNode == '\0')
	{
		std::cout << "Could not find the individual with the name: " << dwarfMeetHouseName << std::endl << std::endl;
		if(dwarfMeetHouseName.compare("Bilbo") != 0) // Bilbo was not entered, check again using Bilbo (NOTICE THE GOTO!!!!!)
		{
			std::cout << "Name entered was not found, and did not exactly match \"Bilbo\". Defaulting to Bilbo" << std::endl;
			dwarfMeetHouseName.assign("Bilbo");
			goto Label_getDwarfMeetLocation;
		}			
		return;
	}
	std::cout << "Routing everyone to " << dwarfMeetHouseName << "'s house" << std::endl << std::endl;
	
	std::vector<NodeValue> dwarfResponses;
	destIndex = (USI)(dwarfMeetNode - 'A');
	
	if(code == ROUTE_MGP)
	{
		dwarfResponses.resize(dwarfs.size());
		
		// We need to brute force MGP as every possible path for each node must be checked, with not guranteed stop (can pass through the destination)
		for(unsigned int i = 0; i < dwarfs.size(); i++) 
		{
			pthread_mutex_lock(&listMutex);
			{
				while(threadList.empty())
					pthread_cond_wait(&stackCond, &listMutex);
				
				MGPThread* mgpT = threadList.front();
				threadList.pop_front();
				mgpT->startMGP(&dwarfResponses, i, (USI)(dwarfs[i].location - 'A'));
			}
			pthread_mutex_unlock(&listMutex);
		}
		
		// Can not progress until all requests have been sent out (handled above) and have returned (handled now)
		pthread_mutex_lock(&listMutex);
		{
			while((short int)threadList.size() != threadCount)
				pthread_cond_wait(&stackCond, &listMutex);
		}
		pthread_mutex_unlock(&listMutex);
	}
	else
	{
		NodeValue responseMap[MAP_SIZE];
		routeDij(responseMap); // Route from dest to all nodes.
		
		for(unsigned int i = 0; i < dwarfs.size(); i++) 
			dwarfResponses.push_back(responseMap[(USI)(dwarfs[i].location - 'A')]);
	}
	
	printResults(&dwarfResponses, &dwarfs, dwarfMeetHouseName);
	
	// Note that we can not delete map memory until after the next requests map is generated, as at that point thread data is also cleared, so they is a temporary memory leak.
}

// This is dijkstras using the destination as the source. 
void routeDij(NodeValue* responseMap)
{	
	responseMap[destIndex].value = 0;
	
	std::list<Node*> nodeList;
	for(int i = 0; i < MAP_SIZE; i++)
		if(map[i] != NULL)
			nodeList.push_back(map[i]);
	
	while(!nodeList.empty())
	{
		// Get least value node
		Node* nodeMin = nodeList.front();
		std::list<Node*>::iterator itErase = nodeList.begin();
		std::list<Node*>::iterator it = nodeList.begin();
		it++;
		for(; it != nodeList.end(); it++)
		{
			if(responseMap[(*it)->locationIndex].value < responseMap[nodeMin->locationIndex].value)
			{
				nodeMin = map[(*it)->locationIndex];
				itErase = it;
			}
		}
		
		responseMap[nodeMin->locationIndex].valuePath.push_back(nodeMin->locationIndex);
		nodeList.erase(itErase);
		for(unsigned int i = 0; i < nodeMin->edges.size(); i++)
		{			
			USI followNodeIndex = followEdgeIndex(nodeMin, nodeMin->edges[i]);
			unsigned int takeEdgeNewTotal = responseMap[nodeMin->locationIndex].value + getEdgevalue(nodeMin->edges[i]);
			
			if(takeEdgeNewTotal < responseMap[followNodeIndex].value)
			{
				responseMap[followNodeIndex].valuePath.clear();
				
				for(unsigned int j = 0; j < responseMap[nodeMin->locationIndex].valuePath.size(); j++)
					responseMap[followNodeIndex].valuePath.push_back(responseMap[nodeMin->locationIndex].valuePath[j]);
				responseMap[followNodeIndex].value = takeEdgeNewTotal;
			}
		}
	}
}

NodeValue routeMGP(bool*** takenMatrix, USI currentNodeIndex, Edge* edgeTaken)
{	
	std::vector<NodeValue> NodeValues; // Each unique next node reachable from this path will insert a response here. We pick which to use via the greed function
	for(unsigned int i = 0; i < map[currentNodeIndex]->edges.size(); i++)
	{
		// Take the edge if it is not taken
		if(!*takenMatrix[currentNodeIndex][i])
		{
			*takenMatrix[currentNodeIndex][i] = true;
			NodeValues.push_back(routeMGP(takenMatrix, followEdgeIndex(map[currentNodeIndex], map[currentNodeIndex]->edges[i]), map[currentNodeIndex]->edges[i]));
		
			// We are back from all of the possible ways taking the above edge can go, so unmark it as taken so the next edge from this node can path back through it, if it is a possible path.
			*takenMatrix[currentNodeIndex][i] = false;
		}
	}

	return greedAlgorithm(edgeTaken, &NodeValues, currentNodeIndex); // Note that edgeTaken can be NULL here!
}

NodeValue greedAlgorithm(Edge* edgeTakenToCurrentNode, std::vector<NodeValue>* valuesForPathsFromCurrentNode, USI currentNodeIndex)
{
	if(valuesForPathsFromCurrentNode->size() > 0)
	{
		NodeValue* bestResponseFromVec = NULL;
		unsigned int i = 0;
		
		// This loop picks the initial response to compare to, must be done like this not picking [0] so that we don't pick a response that doesnt end in the dest accidentily. 
		for(; i < valuesForPathsFromCurrentNode->size() && bestResponseFromVec == NULL; i++)
			if((((*valuesForPathsFromCurrentNode)[i]).valuePath[0] == destIndex))
				bestResponseFromVec = &((*valuesForPathsFromCurrentNode)[i]);
		
		// Pick the best response from the remaining depending on the greed factor
		// MUST END AT DEST, greed method is called end backwards, so this is always possible (also the dest is first index as this is done in reverse!!!)
		for(; i < valuesForPathsFromCurrentNode->size(); i++)
			if(((*valuesForPathsFromCurrentNode)[i].value > bestResponseFromVec->value) && (((*valuesForPathsFromCurrentNode)[i]).valuePath[0] == destIndex))
				bestResponseFromVec = &((*valuesForPathsFromCurrentNode)[i]);
			
		if(bestResponseFromVec != NULL)
		{
			// bestResponseFromVec already has the previous nodes best values, so now we just add our edge values
			bestResponseFromVec->value += getEdgevalue(edgeTakenToCurrentNode);
			bestResponseFromVec->valuePath.push_back(currentNodeIndex);
			return *bestResponseFromVec;
		}
	}
	
	// This case happens at the last node. 
	NodeValue rv;
	rv.value = getEdgevalue(edgeTakenToCurrentNode);
	rv.valuePath.push_back(currentNodeIndex);
	return rv;
}

USI followEdgeIndex(Node* nodeOn, Edge* edgeTake)
{
	if(nodeOn->locationIndex == edgeTake->host1)
		return edgeTake->host2;
	
	return edgeTake->host1;
}

unsigned int getEdgevalue(Edge* edge)
{
	// edge can be NULL!
	if(edge == NULL)
		return 0; // The first node (that dwarf start node) is the only one with a NULL edge, which makes sense as no edge was taken to get there, so the rv does not matter. 
	
	switch(algCode)
	{
		case ROUTE_SHP:
			return 1; // 1 hop per edge
			
		case ROUTE_SDP:
			return edge->distance;
			
		case ROUTE_STP:
			return edge->time;
			
		case ROUTE_FTP:
			return edge->trolls;
			
		case ROUTE_MGP:
			return edge->coins;
	}
	
	return 1; // This should never be reached. 
}

void printResults(std::vector<NodeValue>* dwarfResponses, std::vector<Dwarf>* dwarfs, std::string dwarfMeetHouseName)
{
	switch(algCode)
	{
		case ROUTE_SHP:
			std::cout << "Shortest Hop Path (SHP) Results:" << std::endl;
			break;
			
		case ROUTE_SDP:
			std::cout << "Shortest Distance Path (SDP) Results" << std::endl;
			break;
			
		case ROUTE_STP:
			std::cout << "Shortest Time Path (STP) Results" << std::endl;
			break;
			
		case ROUTE_FTP:
			std::cout << "Fewest Trolls Path (FTP) Results" << std::endl;
			break;
			
		case ROUTE_MGP:
			std::cout << "Most Gold Path (MGP) Results" << std::endl;
			break;
	}
	std::cout << "Destination was " << dwarfMeetHouseName << "'s house" << std::endl << std::endl;
	
	outputFormattedColCentered("Dwarf");
	outputFormattedColCentered("Home");
	outputFormattedColCentered("Hops");
	outputFormattedColCentered("Dist");
	outputFormattedColCentered("Time");
	outputFormattedColCentered("Gold");
	outputFormattedColCentered("Trolls");
	outputFormattedColLeft("      Path");
	std::cout << std::endl;
	for(int i = 0; i < 8; i++)
		for(int j = 0; j < OUTPUT_COL_WIDTH + 2; j++)
			std::cout << '-';
	std::cout << std::endl;
	
	double hopsTotal = 0;
	double distTotal = 0;
	double timeTotal = 0;
	double coinsTotal = 0;
	double trollsTotal = 0;
	
	// The home owner / convergence point person does not get a say in the final average. In mgp they do contribute
	double dwarfCount = (*dwarfResponses).size() + (algCode == ROUTE_MGP ? 0 : -1);
	for(unsigned int d = 0; d < (*dwarfResponses).size(); d++)
	{
		std::string home = "";
		home += (*dwarfs)[d].location;
		int hops = (*dwarfResponses)[d].valuePath.size() - 1;
		int dist = 0;
		int time = 0;
		int coins = 0;
		int trolls = 0;
		std::string path = "      ";
		
		path += (char)('A' + (*dwarfResponses)[d].valuePath[hops]);
		path += ' ';
		for(int i = hops - 1; i >= 0; i--) // Not unsigned, cause if size is 0 than we fault ( 0 - 1 == USINT MAX)
		{
			USI prevNode = (*dwarfResponses)[d].valuePath[i + 1];
			USI onNode = (*dwarfResponses)[d].valuePath[i];
			Edge* edge;
			for(unsigned int e = 0; e < map[prevNode]->edges.size(); e++)
			{
				if(map[prevNode]->edges[e]->host1 == onNode || map[prevNode]->edges[e]->host2 == onNode)
				{
					edge = map[prevNode]->edges[e];
					break;
				}
			}
			
			dist += edge->distance;
			time += edge->time;
			coins += edge->coins;
			trolls += edge->trolls;
			path += (char)('A' + (*dwarfResponses)[d].valuePath[i]);
			path += ' ';
		}
		
		outputFormattedColCentered((*dwarfs)[d].name);
		outputFormattedColCentered(home);
		outputFormattedColCentered(std::to_string(hops));
		outputFormattedColCentered(std::to_string(dist));
		outputFormattedColCentered(std::to_string(time));
		outputFormattedColCentered(std::to_string(coins));
		outputFormattedColCentered(std::to_string(trolls));
		outputFormattedColLeft(path);
		std::cout << std::endl;
		
		hopsTotal += hops;
		distTotal += dist;
		timeTotal += time;
		coinsTotal += coins;
		trollsTotal += trolls;
	}
	
	for(int i = 0; i < 8; i++)
		for(int j = 0; j < OUTPUT_COL_WIDTH + 2; j++)
			std::cout << '-';
	std::cout << std::endl;
	
	outputFormattedColCentered("Average: ");
	outputFormattedColCentered(""); // No average home value
	outputFormattedColCentered(doubleOutput(hopsTotal / dwarfCount));
	outputFormattedColCentered(doubleOutput(distTotal / dwarfCount));
	outputFormattedColCentered(doubleOutput(timeTotal / dwarfCount));
	outputFormattedColCentered(doubleOutput(coinsTotal / dwarfCount));
	outputFormattedColCentered(doubleOutput(trollsTotal / dwarfCount));
	
	std::cout << std::endl;
}

std::string doubleOutput(double d)
{
	// Thanks @https://stackoverflow.com/questions/29200635/convert-float-to-string-with-set-precision-number-of-decimal-digits for formatting decimals
	std::stringstream stream;
	stream << std::fixed << std::setprecision(DOUBLE_STRING_PRECISION) << d;
	return stream.str();
}

void outputFormattedColLeft(std::string value)
{
	std::cout << value;
	for(int i = value.size(); i <= OUTPUT_COL_WIDTH; i++)
		std::cout << ' ';
}

void outputFormattedColCentered(std::string value)
{
	int spaces = (OUTPUT_COL_WIDTH - value.size()) / 2;
	
	for(int i = 0; i < spaces; i++)
		std::cout << ' ';
	
	std::cout << value;
	
	for(int i = value.size() + spaces; i <= OUTPUT_COL_WIDTH; i++)
		std::cout << ' ';
}

void outputFormattedColRight(std::string value)
{
	for(int i = value.size(); i <= OUTPUT_COL_WIDTH; i++)
		std::cout << ' ';
	std::cout << value;
}







