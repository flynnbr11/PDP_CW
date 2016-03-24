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
	for(month = 1 ; month < NUM_MONTHS; month++) {
			sleep(1);
			endMonth();
			printf("month = %d num s = %d \n", month, numLivingSquirrels);			
		}
		//printf("Rank %d shutting down pool \n", rank);
		shutdownPool();
		sleep(2);
		poisonPill(); //Poison cells explicilty after a while
}

void Clock::endMonth(){
	for(int a = 1 + NUM_EXTRA_ACTORS; a <= NUM_EXTRA_ACTORS + NUM_CELLS; a++) {
		MPI_Send(&month, 1, MPI_INT, a, 888, MPI_COMM_WORLD);
	}

	MPI_Status status;
	
	MPI_Send(&month, 1, MPI_INT, TRACKER_RANK, 771, MPI_COMM_WORLD); //ask tracker how many alive
	MPI_Recv(&numLivingSquirrels, 1, MPI_INT, TRACKER_RANK, 772, MPI_COMM_WORLD, &status); // receive from tracker		

}

void Clock::poisonPill() {
	// Send to squirrels; wait a while, send to cells -- CHANGE THIS LATER
	int a;
	sleep(2);
	for(a = NUM_EXTRA_ACTORS + 1; a <= NUM_EXTRA_ACTORS + NUM_CELLS; a++) {
		MPI_Send(&poison, 1, MPI_INT, a, 999, MPI_COMM_WORLD);
		//printf("Sent poison to cell %d \n", a);
	}	
	MPI_Send(&poison, 1, MPI_INT, NURSE_RANK, 999, MPI_COMM_WORLD);
	printf("Sent poison to nurse \n");
}
