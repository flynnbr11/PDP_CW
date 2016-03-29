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
	int flag = 0;
	while(simulationRunning) {
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		if(flag == 1) {			

			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			sender = status.MPI_SOURCE;
			tag = status.MPI_TAG;
			if(tag == 111) { // tag 1 corresponds to squirrel-cell comms
				informSquirrel(sender, tag, status);
			}
			if(tag == 888) { //month
				monthlyUpdates(sender, tag, status);
			}
			if(tag == 999) { // poison pill
				receivePoison(sender, tag, status);
			}
			flag = 0;
		}
	}
}

void Cell::receivePoison (int sender, int tag, MPI_Status status) {
		MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
		simulationRunning = 0;
}



void Cell::informSquirrel(int sender, int tag, MPI_Status status) {
		MPI_Recv(&incomingMessage, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
//		MPI_Ssend(&cellValues, 2, MPI_INT, sender, 112, MPI_COMM_WORLD);	
		MPI_Ssend(&infectionLevel, 1, MPI_INT, sender, 112, MPI_COMM_WORLD);
		MPI_Ssend(&populationInflux, 1, MPI_INT, sender, 113, MPI_COMM_WORLD);
		populationInfluxCurrent++;
		if(incomingMessage == 1) {
			infectionLevelCurrent++;
		}
}



void Cell::monthlyUpdates(int sender, int tag, MPI_Status status) {
	MPI_Recv(&month, 1, MPI_INT, sender, tag, MPI_COMM_WORLD, &status);
	populationInfluxRecentMonths[month%3] = populationInfluxCurrent; // overwrite oldest each time
	infectionLevelRecentMonths[month%2] = infectionLevelCurrent;
	populationInfluxCurrent = 0;
	infectionLevelCurrent = 0;
	infectionLevel = infectionLevelRecentMonths[0] + infectionLevelRecentMonths[1];
	populationInflux = populationInfluxRecentMonths[0] + populationInfluxRecentMonths[1] + populationInfluxRecentMonths[2];
	cellValues[0] = populationInflux;
	cellValues[1] = infectionLevel;
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

void Cell::sendValues() {
	MPI_Ssend(&infectionLevel, 1, MPI_INT, sender, 112, MPI_COMM_WORLD);
	MPI_Ssend(&populationInflux, 1, MPI_INT, sender, 113, MPI_COMM_WORLD);
}

int Cell::getIncomingMessage() {
	return incomingMessage;
}
