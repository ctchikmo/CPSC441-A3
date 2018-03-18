#ifndef MGP_THREAD_H
#define MGP_THREAD_H

#include <pthread.h>

#include "Router.h"

class MGPThread
{
	public:
		MGPThread();
		~MGPThread();
		
		void buildTakenMatrix(Node** map, Node** newMap);
		void startMGP(std::vector<NodeValue>* dwarfResp, USI respPos, USI startPos);
		void stop();
		
	private:
		bool running = true;
		pthread_t thread;
		pthread_mutex_t mutex;
		pthread_cond_t cond;
		
		std::vector<NodeValue>* dwarfResponses = NULL;
		USI responsesPosition = -1;// Max value as unsigned
		USI startPos = -1;// Max value as unsigned
		bool*** takenMatrix = NULL;
		int nodeEdgeSizes[MAP_SIZE];
	
		void runnerLoop();
	
		static void* threadStarter(void* mgpThread);
};

#endif