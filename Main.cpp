#include <iostream>
#include <string>

#include "Router.h"

#define CODE_BAD_INPUT 	0
#define CODE_GOOD 		1
#define CODE_QUIT		2

int handleCommand(std::string input);
void help();

// There are no command line arguments for this program.
int main()
{
	// Start an input loop. This loop takes in commands by the user. 
	std::cout << std::endl << "Please enter a command (enter 'help' to see the command list)" << std::endl << std::endl;
	
	std::string input;
	while(std::getline(std::cin, input))
	{
		int code = handleCommand(input);
		switch(code)
		{
			case CODE_BAD_INPUT:
				std::cout << "Command not recognized..." << std::endl << std::endl;
				break;
				
			case CODE_QUIT:
				std::cout << "Quiting" << std::endl << std::endl;
				return 0;
		}
	}
	
	return 0;
}

int handleCommand(std::string input)
{
	if(input.size() == 1 && input[0] == 'q')
	{
		return CODE_QUIT;
	}
	else if(input.size() == 4 && input[0] == 'h' && input[1] == 'e' && input[2] == 'l' && input[3] == 'p')
	{
		help();
	}
	else if(input.size() == 3)
	{
		if(input[0] == 'a' && input[1] == 'l' && input[2] == 'l')
			route(ROUTE_ALL);
		else if(input[0] == 's' && input[1] == 'h' && input[2] == 'p')
			route(ROUTE_SHP);
		else if(input[0] == 's' && input[1] == 'd' && input[2] == 'p')
			route(ROUTE_SDP);
		else if(input[0] == 's' && input[1] == 't' && input[2] == 'p')
			route(ROUTE_STP);
		else if(input[0] == 'f' && input[1] == 't' && input[2] == 'p')
			route(ROUTE_FTP);
		else if(input[0] == 'm' && input[1] == 'g' && input[2] == 'p')
			route(ROUTE_MGP);
		else
			return CODE_BAD_INPUT;
		
		std::cout << std::endl;
	}
	else
		return CODE_BAD_INPUT;
		
	return CODE_GOOD;
}

void help()
{
	std::cout << "Command List (lowercase only):" << std::endl;
	std::cout << "+ 'help': Brings up this menu" << std::endl;
	std::cout << "+ 'all': Start traversal using all algorithms (asks for file name on use)" << std::endl;
	std::cout << "+ 'shp': Start traversal using the shortest hop path (asks for file name on use)" << std::endl;
	std::cout << "+ 'sdp': Start traversal using the shortest distance path (asks for file name on use)" << std::endl;
	std::cout << "+ 'stp': Start traversal using the shortest time path (asks for file name on use)" << std::endl;
	std::cout << "+ 'ftp': Start traversal using the fewest trolls path (asks for file name on use)" << std::endl;
	std::cout << "+ 'mgp': Start traversal using the maximum gold path (asks for file name on use)" << std::endl;
	std::cout << std::endl;
}





