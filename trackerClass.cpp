#include <stdio.h>
#include "mpi.h"
#include "pool.h"
#include "squirrel-functions.h"
#include "ran2.h"
#include <iostream>
#include "parameters.h" 

using namespace std;
#include "trackerClass.h"




Tracker::Tracker() {
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	state = - 1 - rank;
	numLivingSquirrels = NUM_SQUIRRELS;
}


void Tracker::counter() {
	int sender;
	int tag;
	int flag =0;
	int incomingMessage;
	MPI_Status status;
	while(shouldWorkerStop() == 0) {
		flag =0;
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		if(flag == 1) {			
			sender = status.MPI_SOURCE;
			tag = status.MPI_TAG;
			printf("tracker %d receiving from %d tag = %d \n", rank, sender, tag);
			
			if(tag == 777) {
				MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
				printf("S %d will die; received on %d \n", sender, rank);
				numLivingSquirrels --;
			}

			else if(tag == 778) {
				MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
				printf("New squirrel born with workerPid %d \n", incomingMessage);
				numLivingSquirrels ++;
				if(numLivingSquirrels > 100) {
					shutdownPool();
					printf("Tracker %d shutting down pool b/c num sq = %d \n", numLivingSquirrels);
				}
			}
			
			else if(tag == 771) { //clock wants to know number living
				MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);						
				MPI_Send(&numLivingSquirrels, 1, MPI_INT, sender, 772, MPI_COMM_WORLD);
			}
		}
	}
	printf("tracker outside while loop; stop = 1, num squirrels = %d \n", numLivingSquirrels);
}


