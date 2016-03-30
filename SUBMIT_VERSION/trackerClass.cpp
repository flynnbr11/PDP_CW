#include <stdio.h>
#include "mpi.h"
#include "pool.h"
#include "squirrel-functions.h"
#include "ran2.h"
#include <iostream>
#include "parameters.h" 

using namespace std;
#include "trackerClass.h"

/*
* Tracker waiting on messages for duration of simulation. 
* Possible messages:
* 	- Squirrel has died
*		- Squirrel has been born
* 	- Squirrel has caught infection
* 	- Squirrel has shut down
* 	- Clock has finished month
* Tracker resposibilities:
* 	- Tracker maintains the total number of living/infected squirrels at all times. 
* 	- Send clock these values monthly
* 	- When simulation shutting down, ensure all squirrels have shut down before
*				telling clock to continue shut down. 
*/


Tracker::Tracker() {
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	state = - 1 - rank;
	numLivingSquirrels = NUM_SQUIRRELS;
	numInfectedSquirrels = NUM_INITIAL_INFECTION;
	for (int i =0 ; i< NUM_CELLS + NUM_EXTRA_ACTORS + NUM_SQUIRRELS + MAX_NUM_SQUIRRELS; i++){
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
			// Squirrel has died
			if(tag == 777) {
				deathMessage(sender, tag, status);
			}

			// squirrel has become infected
			else if(tag == 776) { 
				squirrelInfectedMessage(sender, tag, status);
			}
			
			// Squirrel has given birth
			else if(tag == 778) {
				birthMessage(sender,tag,status);
			}
			
			// Clock has ended month
			else if(tag == 771) { 
				endOfMonth(sender, tag, status);
			}
			
			// Squirrel has shut down
			else if(tag == 773) { //squirrels are shutting down
				shutDownMessage(sender, tag, status); //changes continueSim when ready
			}
		}
	}
}


/*
* Functions based on the decoded tag. 
* Function names self explanatory.
*/



void Tracker::squirrelInfectedMessage(int sender, int tag, MPI_Status status) {
	MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
	numInfectedSquirrels++;				
}


void Tracker::birthMessage(int sender, int tag, MPI_Status status) {
	MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
	numLivingSquirrels ++;
	// If a new process initiated, update array so we have total count of active ranks
	if(squirrelArray[incomingMessage] == 0) {
		squirrelArray[incomingMessage] = 1; 
	} 
	// Print the below statement for output every population change.
	// printf("Squirrel %d has given birth to rank %d, pop now = %d \n", sender, incomingMessage, numLivingSquirrels);
}


/*
* Receive message of death. Update values of living/infected squirrels.
*/
void Tracker::deathMessage(int sender, int tag, MPI_Status status) {
	MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
	numLivingSquirrels --;
	numInfectedSquirrels--;
	// Print the below statement for output every population change.
	// printf("Squirrel %d has died, pop now = %d \n", sender, numLivingSquirrels);
}

/*
* Receive message stating month has ended.
* Send back numbers of living & infected squirrels in an array. 
*/
void Tracker::endOfMonth(int sender, int tag, MPI_Status status){
	int numLivingArray[2];
	numLivingArray[0] = numLivingSquirrels;
	numLivingArray[1] = numInfectedSquirrels;
	MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);						
	MPI_Ssend(&numLivingArray[0], 2, MPI_INT, sender, 772, MPI_COMM_WORLD);
}


/*
* Find rank of squirrel which is shutting down. 
* When all active processes have returned, send message to 
* 	clock to allow it to finish shutting down simulation. 
*/
void Tracker::shutDownMessage(int sender, int tag, MPI_Status status) {
	
	MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);						
	squirrelArray[sender] = 0;
	squirrelSum = 0;
	for (int i =0; i< NUM_SQUIRRELS + MAX_NUM_SQUIRRELS; i++){
		squirrelSum += squirrelArray[i];
	}

	if(squirrelSum == 0 ) {
		MPI_Ssend(&numLivingSquirrels, 1, MPI_INT, CLOCK_RANK, 774, MPI_COMM_WORLD);
		printf("All squirrels have shut down. Tracker shutting down \n");
		continueSim = 0;			
	}
}


