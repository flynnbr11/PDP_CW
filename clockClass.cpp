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
			sleep(3);
			endMonth();
			printf("month = %d \n", month);			
		}
		poisonPill();
}

void Clock::endMonth(){
	for(int a=1+NUM_EXTRA_ACTORS; a <= NUM_EXTRA_ACTORS + NUM_CELLS; a++) {
		MPI_Send(&month, 1, MPI_INT, a, 888, MPI_COMM_WORLD);
	}
}


void Clock::poisonPill() {
	// Send to squirrels; wait a while, send to cells -- CHANGE THIS LATER
	int a;
	for(a = NUM_EXTRA_ACTORS + NUM_CELLS + 1; a <= NUM_EXTRA_ACTORS + NUM_CELLS + NUM_SQUIRRELS; a++) {
		MPI_Send(&poison, 1, MPI_INT, a, 999, MPI_COMM_WORLD);
	}
	sleep(2);
	for(a = 1 + NUM_EXTRA_ACTORS; a <= NUM_EXTRA_ACTORS + NUM_CELLS; a++) {
		MPI_Send(&poison, 1, MPI_INT, a, 999, MPI_COMM_WORLD);
	}	
}
