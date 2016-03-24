#include <stdio.h>
#include "mpi.h"
#include "pool.h"
#include "squirrel-functions.h"
#include "ran2.h"
#include <iostream>
#include "parameters.h" 

using namespace std;
#include "nurseClass.h"


Nurse::Nurse() {
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	simulationRunning = 1;
}

void Nurse::run() {
	//printf("i am nurse \n");
}

void Nurse::newSquirrel() {
	int flag = 0;
	int sender, tag;
	MPI_Status status;
	float pos[2],x,y;
	int incomingMessage;
	while(simulationRunning == 1) {
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		if(flag == 1) {			
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			sender = status.MPI_SOURCE;
			tag = status.MPI_TAG;
		
			if(tag == 222) {
				MPI_Recv(&pos[0], 2, MPI_FLOAT, sender, tag, MPI_COMM_WORLD, &status);		
				x = pos[0];
				y = pos[1];
				printf("nurse %d received from s %d to birth on x=%f y=%f \n", rank, sender,x,y);
				birthNewSquirrel();
			}
			
			else if(tag == 999) {
				printf("Nurse received poison \n");
				MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);				
				simulationRunning = 0;
			}
			flag = 0;
		}
	}
}

void Nurse::birthNewSquirrel() {
	int workerPid = 1;
	workerPid = startWorkerProcess();
	MPI_Send(&workerPid, 1, MPI_INT, TRACKER_RANK, 778, MPI_COMM_WORLD);
}
