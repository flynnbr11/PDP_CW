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

Clock::Clock() {
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	month = 0;
	monthOver = 888;
	poison = 999;
}

void Clock::timer() {
	int incomingMessage;
	int flag =0 ;
	MPI_Status status;
	for(month = 1 ; month <= NUM_MONTHS; month++) {
		sleep(SECONDS_PER_MONTH);
		endMonth();
		printf("Month = %d. Number of squirrels = %d Number Infected = %d \n", month, numLivingSquirrels, numInfectedSquirrels);			
	}
	shutdownPool();

	MPI_Recv(&numLivingSquirrels, 1, MPI_INT, TRACKER_RANK, 774, MPI_COMM_WORLD, &status); // receive from tracker		
	printf("At end of simulation: %d Squirrels alive\n", numLivingSquirrels);
	poisonPill(); //Poison cells explicilty after a while

	
}

void Clock::endMonth(){
	for(int a= 1 + NUM_EXTRA_ACTORS; a <= NUM_EXTRA_ACTORS + NUM_CELLS; a++) {
		MPI_Ssend(&month, 1, MPI_INT, a, 888, MPI_COMM_WORLD);
	}

	int incomingMessage[3];
	MPI_Ssend(&month, 1, MPI_INT, TRACKER_RANK, 771, MPI_COMM_WORLD); //ask tracker how many alive
	MPI_Recv(&incomingMessage[0], 3, MPI_INT, TRACKER_RANK, 772, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive from tracker		
	numLivingSquirrels = incomingMessage[0]; 
	numInfectedSquirrels = incomingMessage[1];
	if(incomingMessage[2] == 1) { //this means too many squirrels are active
		printf("Clock shutting down early %d squirrels alive \n", numLivingSquirrels);
		month = NUM_MONTHS;
	}
}

void Clock::poisonPill() {
	int a;
	sleep(2);
	for(a = NUM_EXTRA_ACTORS + 1; a <= NUM_EXTRA_ACTORS + NUM_CELLS; a++) {
		MPI_Ssend(&poison, 1, MPI_INT, a, 999, MPI_COMM_WORLD);
	}	
}
