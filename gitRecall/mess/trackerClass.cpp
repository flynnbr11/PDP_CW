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
	for (int i =0 ; i< NUM_SQUIRRELS + MAX_NUM_SQUIRRELS; i++){
		squirrelArray[i] = 0;
	}
	for( int i = NUM_EXTRA_ACTORS + NUM_CELLS + 1; i < NUM_EXTRA_ACTORS + NUM_CELLS + NUM_SQUIRRELS; i++) {
		squirrelArray[i] = 1;
	}
}


void Tracker::counter() {
	int sender;
	int tag;
	int flag =0;
	continueSim = 1;
	MPI_Status status;
	
	while(continueSim) {
		flag =0;
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		if(flag == 1) {			
			sender = status.MPI_SOURCE;
			tag = status.MPI_TAG;
			printf("Tracker received tag %d \n", tag);
			if(tag == 777) {
				deathMessage(sender, tag, status);
			}

			else if(tag == 778) {
				birthMessage(sender,tag,status);
			}
			
			else if(tag == 771) { //clock wants to know number living
				endOfMonth(sender, tag, status);
			}

			else if(tag == 773) { //squirrels are shutting down
				shutDownMessage(sender, tag, status); //changes continueSim when ready
			}
		}
	}
}

void Tracker::birthMessage(int sender, int tag, MPI_Status status) {
	MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
	numLivingSquirrels ++;
	squirrelArray[incomingMessage] = 1; //this is the rank of the newborn
	printf("Squirrel %d has given birth to rank %d, pop now = %d \n", sender, incomingMessage, numLivingSquirrels);
}

void Tracker::deathMessage(int sender, int tag, MPI_Status status) {
	MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
	numLivingSquirrels --;
	printf("Squirrel %d has died, pop now = %d \n", sender, numLivingSquirrels);
	squirrelArray[sender] = 0;
}


void Tracker::endOfMonth(int sender, int tag, MPI_Status status){
	int numLivingArray[2];
	numLivingArray[0] = numLivingSquirrels;
	numLivingArray[1] = 0; //will be 1 if too many squirrels and should shut down
	if(numLivingSquirrels > MAX_NUM_SQUIRRELS || numLivingSquirrels <= 0 ) {
		numLivingArray[1] = 1;					
	}

	MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);						
	MPI_Ssend(&numLivingArray[0], 2, MPI_INT, sender, 772, MPI_COMM_WORLD);
}

void Tracker::shutDownMessage(int sender, int tag, MPI_Status status) {
	
	MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);						
	squirrelArray[sender] = 0;
	squirrelSum = 0;
	for (int i =0 ; i< NUM_SQUIRRELS + MAX_NUM_SQUIRRELS; i++){
		squirrelSum += squirrelArray[i];
		if(squirrelArray[i] == 1) printf("Squirrel Array = %d; S %d still alive \n", squirrelSum, i);
	}
	printf("Tracker received shut down message from S %d\n", sender);

	if(squirrelSum ==0 ) {
		MPI_Ssend(&numLivingSquirrels, 1, MPI_INT, CLOCK_RANK, 774, MPI_COMM_WORLD);
		printf("All squirrels have shut down. Tracker shutting down \n");
		continueSim = 0;			
	}
}


