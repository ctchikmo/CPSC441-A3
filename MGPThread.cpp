#include <iostream>

#include "MGPThread.h"

MGPThread::MGPThread()
{
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
	
	int rv = pthread_create(&thread, NULL, threadStarter, this);
	if (rv != 0)
	{
		std::cout << "Error creating MGPThread's thread, exiting program" << std::endl;
		exit(-1);
	}
}

MGPThread::~MGPThread()
{
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
}

void MGPThread::buildTakenMatrix(Node** map)
{
	// Handle map deletion of the map created for the thread, if a previous topology map was read. It is only NULL on the first run
	/*
	if(takenMatrix != NULL)
	{
		for(unsigned int i = 0; i < MAP_SIZE; i++)
			delete[] takenMatrix[i];
		
		delete[] takenMatrix;
	}
	*/
	
	takenMatrix = new bool**[MAP_SIZE];
	for(int i = 0; i < MAP_SIZE; i++)
	{
		if(map[i] != NULL)
		{
			int size = map[i]->edges.size();
			takenMatrix[i] = new bool*[size];
			
			// Nodes share edges, so must have 2 linking nodes point at same bool
			for(int j = 0; j < size; j++)
			{
				USI otherNodeIndex = followEdgeIndex(map[i], map[i]->edges[j]);// This method is in Router.cpp
				if(otherNodeIndex > map[i]->locationIndex) // If the index is greater than the other nodes edge taken list is not yet set
				{
					takenMatrix[i][j] = new bool;
					*takenMatrix[i][j] = false;
				}
				else
				{
					for(unsigned int k = 0; k < map[otherNodeIndex]->edges.size(); k++)
					{
						if(map[otherNodeIndex]->edges[k]->host1 == i || map[otherNodeIndex]->edges[k]->host2 == i)
						{
							takenMatrix[i][j] = takenMatrix[otherNodeIndex][k];
							break;
						}
					}
				}
			}
		}
	}
}

void MGPThread::startMGP(std::vector<NodeValue>* dwarfResp, USI respPos, USI sp)
{
	pthread_mutex_lock(&mutex);
	{
		dwarfResponses = dwarfResp;
		responsesPosition = respPos;
		startPos = sp;
	
		pthread_cond_signal(&cond);
	}
	pthread_mutex_unlock(&mutex);
}

void MGPThread::runnerLoop()
{
	while(running)
	{
		pthread_mutex_lock(&mutex);
		{
			mgpFinished(this);
			
			while(dwarfResponses == NULL)
				pthread_cond_wait(&cond, &mutex);
		}
		pthread_mutex_unlock(&mutex);
		
		// At this point we have a valid mgp start value to run with
		std::cout << (char)(startPos + 'A') << "::start" << std::endl;
		NodeValue resp = routeMGP(takenMatrix, startPos, NULL); // routeMGP is located in Router.cpp
		std::cout << (char)(startPos + 'A') << "::end" << std::endl;
		(*dwarfResponses)[responsesPosition] = resp;
		
		dwarfResponses = NULL;
		responsesPosition = -1;
		startPos = -1;
	}
}

void MGPThread::stop()
{
	pthread_mutex_lock(&mutex);
	{
		running = false;
		pthread_cond_signal(&cond);
	}
	pthread_mutex_unlock(&mutex);
}

void* MGPThread::threadStarter(void* mgpThread)
{
	MGPThread* mgp = (MGPThread*)mgpThread;
	mgp->runnerLoop();
	
	delete mgp; // thread is created dynamically, and handles its own deletion
	return NULL;
}





