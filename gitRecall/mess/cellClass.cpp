#include <stdio.h>
#include "mpi.h"
#include "pool.h"
#include "squirrel-functions.h"
#include "ran2.h"
#include <iostream>
#include "parameters.h" 

using namespace std;
#include "cellClass.h"


Cell::Cell() {
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
}

void Cell::run() {
	simulationRunning = 1;
	populationInfluxCurrent = 0;
	infectionLevelCurrent = 0;
	populationInflux = 0;
	infectionLevel = 0;
	for(int i =0; i < 2; i++){
		populationInfluxRecentMonths[i] = 0;
		infectionLevelRecentMonths[i] = 0;
	}
	populationInfluxRecentMonths[2] = 0;
	
}


int Cell::getRank() {
	return rank;
}

void Cell::receiveInfo() {
	MPI_Status status;
	MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	sender = status.MPI_SOURCE;
	tag = status.MPI_TAG;
//	MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
	if(tag == 111) { // tag 1 corresponds to squirrel-cell comms
		MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
		//printf("cell mpi_send 1 \n");
		MPI_Ssend(&infectionLevel, 1, MPI_INT, sender, 112, MPI_COMM_WORLD);
		//printf("cell mpi_rec 1 \n");
		//printf("cell mpi_send 2 \n");
		MPI_Ssend(&populationInflux, 1, MPI_INT, sender, 113, MPI_COMM_WORLD);
		//printf("cell mpi_rec 2 \n");

//		sendValues(); 
		populationInfluxCurrent++;
		if(incomingMessage == 1) {
			infectionLevelCurrent++;
		}
	//printf("Cell %d has pop %d and inf %d \n", rank, populationInflux, infectionLevel);
	}
	if(tag == 888) {
		MPI_Recv(&month, 1, MPI_INT, sender, 888, MPI_COMM_WORLD, &status);
	//	//printf("Received MONTH --- cell %d \n", rank);
		monthlyUpdates();
		
	}
	if(tag == 999) { // poison pill
		MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
	//	//printf("Received poison cell %d \n", rank);
		simulationRunning = 0;
	}
}

void Cell::monthlyUpdates() {
	populationInfluxRecentMonths[month%3] = populationInfluxCurrent; // overwrite oldest each time
	infectionLevelRecentMonths[month%2] = infectionLevelCurrent;
	populationInfluxCurrent = 0;
	infectionLevelCurrent = 0;
	infectionLevel = infectionLevelRecentMonths[0] + infectionLevelRecentMonths[1];
	populationInflux = populationInfluxRecentMonths[0] + populationInfluxRecentMonths[1] + populationInfluxRecentMonths[2];
}

int Cell::getInfectionLevel() {
	return infectionLevel;
}
int Cell::getPopulationInflux() {
	return populationInflux;
}

int Cell::getSimulation() {
	return simulationRunning;
}	


void Cell::receiveWhileRunning() {
	int count = 0;
	int flag = 0;
	MPI_Status status;
	while(simulationRunning) { //while simulation in process - global variable?
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		if(flag == 1) {			
			receiveInfo();
			flag = 0;
		}
		count ++;
	}
}

void Cell::printStatus() {
//	//printf("printStatus: Cell %d has populationInflux %d and infectionLevel %d \n", rank, populationInfluxCurrent, infectionLevelCurrent);
}

void Cell::sendValues() {
	//printf("cell mpi_send 3 \n");
	MPI_Ssend(&infectionLevel, 1, MPI_INT, sender, 112, MPI_COMM_WORLD);
	//printf("cell mpi_rec 3 \n");
	//printf("cell mpi_send 4 \n");
	MPI_Ssend(&populationInflux, 1, MPI_INT, sender, 113, MPI_COMM_WORLD);
	//printf("cell mpi_rec 4 \n");
}

int Cell::getIncomingMessage() {
	return incomingMessage;
}
