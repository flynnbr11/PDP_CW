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
		sendValues(); 
		populationInfluxCurrent++;
		if(incomingMessage == 1) {
			infectionLevelCurrent++;
		}
	}
	if(tag == 888) {
		MPI_Recv(&month, 1, MPI_INT, 0, 888, MPI_COMM_WORLD, &status);
		monthlyUpdates();
		
	}
	if(tag == 999) { // poison pill
		MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
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
//	printf("printStatus: Cell %d has populationInflux %d and infectionLevel %d \n", rank, populationInfluxCurrent, infectionLevelCurrent);
}

void Cell::sendValues() {
	MPI_Send(&infectionLevel, 1, MPI_INT, sender, 112, MPI_COMM_WORLD);
	MPI_Send(&populationInflux, 1, MPI_INT, sender, 113, MPI_COMM_WORLD);
}

int Cell::getIncomingMessage() {
	return incomingMessage;
}
