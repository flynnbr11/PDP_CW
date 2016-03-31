#include <stdio.h>
#include "mpi.h"
#include "pool.h"
#include "squirrel-functions.h"
#include "ran2.h"
#include <iostream>
#include "parameters.h" 
#include "unistd.h"
using namespace std;
#include "clockClass.h"
#include "parameters.h" 


/*
* The clock is responsible for administering the simulation. 
* 	- Simulating months
* 	- Telling cells at the end of each month
*	 	- Printing number of alive/infected squirrels monthly
* 	- Shutting down when time is up or too many/few squirrels to continue
*/



Clock::Clock() {
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
}
void Clock::timer() {
	int incomingMessage;
	int flag =0 ;
	MPI_Status status;
	
	// Time simulator 
	for(month = 1 ; month <= NUM_MONTHS; month++) {
		sleep(SECONDS_PER_MONTH);
		endMonth();
		printf("Month = %d. Number of squirrels = %d Number Infected = %d \n", month, numLivingSquirrels, numInfectedSquirrels);			
	}
	// Start shut down procedure
	shutdownPool();
	// Wait for tracker to say all squirrels have shut down
	MPI_Recv(&numLivingSquirrels, 1, MPI_INT, TRACKER_RANK, 774, MPI_COMM_WORLD, &status); // receive from tracker		

	// Tell cells to shut down now that all squirrels are shut down	
	poisonPill(); 
	printf("At end of simulation: %d Squirrels alive\n", numLivingSquirrels);

}

/*
* Actions to take after each month has finished. 
* 	- Tell cells month has ended 
* 	- Receive number living/infected at end of that month
* 	- Check if number of squirrels is too high or zero
*/
void Clock::endMonth(){
	for(int a= 1 + NUM_EXTRA_ACTORS; a <= NUM_EXTRA_ACTORS + NUM_CELLS; a++) {
		MPI_Ssend(&month, 1, MPI_INT, a, 888, MPI_COMM_WORLD);
	}

	int incomingMessage[2];
	MPI_Ssend(&month, 1, MPI_INT, TRACKER_RANK, 771, MPI_COMM_WORLD); //ask tracker how many alive
	MPI_Recv(&incomingMessage[0], 2, MPI_INT, TRACKER_RANK, 772, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive from tracker		
	numLivingSquirrels = incomingMessage[0]; 
	numInfectedSquirrels = incomingMessage[1];
	// Check if sim should stop b/c of number of squirrels 
	if(numLivingSquirrels > MAX_NUM_SQUIRRELS || numLivingSquirrels < 1) {	
		if(numLivingSquirrels > MAX_NUM_SQUIRRELS)printf("Simulation shutting down after %d months because number of squirrels too high.\n", month);
		if(numLivingSquirrels < 1)printf("Simulation shutting down after %d months because all squirrels have died. \n", month);
		month = NUM_MONTHS; // to exit for loop simulating time
	}
}

/*
* Tell cells to shut down. 
*/
void Clock::poisonPill() {
	int a, incomingMessage;
	sleep(2);
	for(a = NUM_EXTRA_ACTORS + 1; a <= NUM_EXTRA_ACTORS + NUM_CELLS; a++) {
		MPI_Ssend(&incomingMessage, 1, MPI_INT, a, 999, MPI_COMM_WORLD);
	}	
}
