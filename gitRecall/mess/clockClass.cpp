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
	for(month = 1 ; month < NUM_MONTHS; month++) {
		sleep(1);
		endMonth();
		printf("month = %d num squirrels = %d \n", month, numLivingSquirrels);			
	}
//		if(shouldWorkerStop()==0) {
//			printf("clock Rank %d shutting down pool \n", rank);
//			shutdownPool();
//		}
	shutdownPool();

	MPI_Recv(&incomingMessage, 1, MPI_INT, TRACKER_RANK, 774, MPI_COMM_WORLD, &status); // receive from tracker		
	printf("Received kill from tracker \n");
	printf("Starting poison pill \n");
	poisonPill(); //Poison cells explicilty after a while

	
}

void Clock::endMonth(){
	for(int a= 1 + NUM_EXTRA_ACTORS; a <= NUM_EXTRA_ACTORS + NUM_CELLS; a++) {
		printf("clock mpi_send 1 \n");
		MPI_Ssend(&month, 1, MPI_INT, a, 888, MPI_COMM_WORLD);
		printf("clock mpi_rec 1 \n");
		printf("Sent end of month %d to cell %d \n", month, a);
	}
	MPI_Status status;
	int flag =0 ;
	int incomingMessage[2];
	printf("clock mpi_send 2 \n");
	MPI_Ssend(&month, 1, MPI_INT, TRACKER_RANK, 771, MPI_COMM_WORLD); //ask tracker how many alive
	printf("clock mpi_rec 2 \n");
	MPI_Recv(&incomingMessage[0], 2, MPI_INT, TRACKER_RANK, 772, MPI_COMM_WORLD, &status); // receive from tracker		
	numLivingSquirrels = incomingMessage[0]; 
	if(incomingMessage[1] == 1) {
		printf("Clock shutting down early bc too many squirrels = %d \n", numLivingSquirrels);
		month = NUM_MONTHS;
	}
}

void Clock::poisonPill() {
	// Send to squirrels; wait a while, send to cells -- CHANGE THIS LATER
	int a;
	sleep(2);
	for(a = NUM_EXTRA_ACTORS + 1; a <= NUM_EXTRA_ACTORS + NUM_CELLS; a++) {
		printf("clock mpi_send 3 \n");
		MPI_Ssend(&poison, 1, MPI_INT, a, 999, MPI_COMM_WORLD);
		printf("clock mpi_rec 3 \n");
		printf("Sent poison to cell %d \n", a);
	}	
	sleep(1);
	printf("sending poison to tracker %d \n", TRACKER_RANK);
	printf("clock mpi_send 4 \n");
//	MPI_Ssend(&poison, 1, MPI_INT, TRACKER_RANK, 999, MPI_COMM_WORLD);
	printf("clock mpi_rec 4 \n");
}
