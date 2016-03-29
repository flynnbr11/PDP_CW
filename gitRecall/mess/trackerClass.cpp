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
	int onlyOnce =0;
	int incomingMessage;
	int numShutDownSquirrels = 0;
	int continueSim = 1;
	MPI_Status status;
	int squirrelArray[NUM_SQUIRRELS + MAX_NUM_SQUIRRELS];
	int squirrelSum;
	for (int i =0 ; i< NUM_SQUIRRELS + MAX_NUM_SQUIRRELS; i++){
		squirrelArray[i] = 0;
	}
	for( int i = NUM_EXTRA_ACTORS + NUM_CELLS + 1; i < NUM_EXTRA_ACTORS + NUM_CELLS + NUM_SQUIRRELS; i++) {
		squirrelArray[i] = 1;
	}
	
	while(continueSim) {
//		//printf("numShut = %d numLiving = %d \n", numShutDownSquirrels, numLivingSquirrels);
		flag =0;
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		if(flag == 1) {			
			sender = status.MPI_SOURCE;
			tag = status.MPI_TAG;
			printf("tracker %d receiving from %d tag = %d \n", rank, sender, tag);
			
			if(tag == 777) {
				MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
				//printf("S %d will die; received on %d \n", sender, rank);
				numLivingSquirrels --;
				printf("Squirrel %d has died, pop now = %d \n", sender, numLivingSquirrels);
				squirrelArray[sender] = 0;
			}

			else if(tag == 778) {
				MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
				//printf("New squirrel born with workerPid %d by parent %d \n", incomingMessage, sender);
				numLivingSquirrels ++;
				squirrelArray[incomingMessage] = 1;
				printf("Squirrel %d has given birth to rank %d, pop now = %d \n", sender, incomingMessage, numLivingSquirrels);
//				if(numLivingSquirrels > MAX_NUM_SQUIRRELS && onlyOnce == 0) {
//					//printf("tracker mpi_send 0 \n");			
//					MPI_Ssend(&numLivingSquirrels, 1, MPI_INT, CLOCK_RANK, 799, MPI_COMM_WORLD);
//					//printf("tracker mpi_rec 0 \n");
//					//shutdownPool();
//					//printf("Tracker %d shutting down pool b/c num sq = %d \n",rank, numLivingSquirrels);
//					onlyOnce = 1;
//				}
			}
			
			else if(tag == 771) { //clock wants to know number living
				int numLivingArray[2];
				numLivingArray[0] = numLivingSquirrels;
				numLivingArray[1] = 0; //will be 1 if too many squirrels and should shut down
				if(numLivingSquirrels > MAX_NUM_SQUIRRELS) {
					//printf("tracker says Too many squirrels \n");
					numLivingArray[1] = 1;					
				}
				
				MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);						
				//printf("tracker mpi_send 1 \n");			
				MPI_Ssend(&numLivingArray[0], 2, MPI_INT, sender, 772, MPI_COMM_WORLD);
				//printf("tracker mpi_rec 1 \n");
			}

			else if(tag == 773) { //squirrels are shutting down
				MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);						
				numShutDownSquirrels ++;	
//				numLivingSquirrels --;
				squirrelArray[sender] = 0;
				squirrelSum = 0;
				for (int i =0 ; i< NUM_SQUIRRELS + MAX_NUM_SQUIRRELS; i++){
					squirrelSum += squirrelArray[i];
					if(squirrelArray[i] == 1) printf("S %d still alive", i);
				}
				printf("Tracker received shut down message from S %d\n", sender);
				//printf("Tracker: numShutDownSquirrels = %d  numLiving = %d squirrelSum = %d\n", numShutDownSquirrels, numLivingSquirrels, squirrelSum);
//				if(numShutDownSquirrels ==  numLivingSquirrels  ) {
//				if(  numLivingSquirrels < 1  ) {
				if(squirrelSum ==0 ) {
					
					//printf("tracker mpi_send 2 \n");			
					MPI_Ssend(&numLivingSquirrels, 1, MPI_INT, CLOCK_RANK, 774, MPI_COMM_WORLD);
					//printf("tracker mpi_rec 2 \n");
//					MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);						
//					//printf("Tracker received poison, numLive = %d numShut = %d \n", numLivingSquirrels, numShutDownSquirrels);
					continueSim = 0;			
					//printf("tracker: CONTINUE SIM = 0 \n");
				}
			}
			
//			else if(tag == 999) {
//				//printf("tracker going to get poisoned \n");
//				MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);						
//				//printf("Tracker received poison, numLive = %d numShut = %d \n", numLivingSquirrels, numShutDownSquirrels);
//				continueSim = 0;			
//			}
		
		}
	}
	//printf("tracker outside while loop; stop = 1, num squirrels = %d \n", numLivingSquirrels);
}


