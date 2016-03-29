#include <stdio.h>
#include "mpi.h"
#include "pool.h"
#include "squirrel-functions.h"
#include "ran2.h"
#include <iostream>
#include "parameters.h" 

using namespace std;
#include "squirrelClass.h"

Squirrel::Squirrel(){
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	state = - 1 - rank;
	initialiseRNG(&state);
}

int Squirrel::getRank() {
	return rank;
}

int Squirrel::getCellRank() {
	return cellRank;
}

void Squirrel::run() {
	x=0;
	y=0;
	infected = 0;
	simulationRunning = 1;
	numSteps = 0;
	if( rank % 4 == 0) { //give four random squirrels infection

		infected = 1;
		infectedStep = 0;
	}
//	if(getCommandData() != 0 ) {
//		int parent = getCommandData();
//		printf("I am newborn  on rank %d. My parent was %d.", rank, parent);
//		MPI_Recv(&pos[0], 2, MPI_FLOAT, parent, 115, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//		x = pos[0]; 
//		y = pos[1];
//		printf("Newborn %d. My starting position is x = %f y=%f \n", rank, x,y);
//	}
	printf("S %d my parent was %d \n", rank, getCommandData());
	for(int i = 0; i<50; i++) {
		infectionRecentCells[i]=0;
		populationRecentCells[i]=0;
	}

}

void Squirrel::getSquirrelCell() {
	cellRank = 1 + NUM_EXTRA_ACTORS + getCellFromPosition(x,y);
}

void Squirrel::squirrelToCell() {
	MPI_Status status;
	MPI_Ssend(&infected, 1, MPI_INT, cellRank, 111, MPI_COMM_WORLD);
	MPI_Recv(&infectionCurrentCell, 1, MPI_INT, cellRank, 112, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
	MPI_Recv(&populationCurrentCell, 1, MPI_INT, cellRank, 113, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
	populationRecentCells[numSteps % 50 ] = populationCurrentCell;
	infectionRecentCells[numSteps % 50] = infectionCurrentCell;
//	MPI_Recv(&cellValues[0], 2, MPI_INT, cellRank, 112, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
//	populationRecentCells[numSteps % 50 ] = cellValues[0];
//	infectionRecentCells[numSteps % 50] = cellValues[1];
		
}

void Squirrel::receive() {
	MPI_Status status;
	int sender=-1;
	MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
	if(flag == 1) {
		sender = status.MPI_SOURCE;
		tag = status.MPI_TAG;

		if(tag == 999) {
			MPI_Recv(&simulationRunning, 1, MPI_INT, sender, 999, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			simulationRunning = 0;
		}
		flag = 0;	
	} 
}


void Squirrel::updateStep() {
	float x_new, y_new;
	squirrelStep(x,y,&x_new,&y_new,&state); //initilise squirrel positions	
	x=x_new;
	y=y_new;	
	numSteps ++;
	pos[0] = x;
	pos[1] = y;
}


// receive function deprecated
void Squirrel::receiveInfectionLevel() {
	MPI_Recv(&infectionCurrentCell, 1, MPI_INT, cellRank, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Recv(&populationCurrentCell, 1, MPI_INT, cellRank, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	populationRecentCells[numSteps % 50 ] = populationCurrentCell;
	infectionRecentCells[numSteps % 50] = infectionCurrentCell;
}

int Squirrel::getSimulation() {
	return simulationRunning;
}

void Squirrel::updateValues() {
	avgInfLevel = 0;
	avgPopInflux = 0;
	for(int i = 0; i < 50; i++) { //could be done more efficiently
		avgInfLevel += infectionRecentCells[i];
		avgPopInflux += populationRecentCells[i];
	}
	avgInfLevel = avgInfLevel/50;
	avgPopInflux = avgPopInflux/50;
}

void Squirrel::giveBirth() {
	int workingPid;
	birth = 0;
	if(numSteps % 50 == 0) {
		birth = willGiveBirth(avgPopInflux, &state);
		if(birth == 1 && shouldWorkerStop() == 0) {
			workingPid = startWorkerProcess();
			MPI_Ssend(&workingPid, 1, MPI_INT, TRACKER_RANK, 778, MPI_COMM_WORLD); //tell tracker of birth - send childid, x,y
//			printf("S %d before sending pos to child %d \n", rank, workingPid);
//			MPI_Ssend(&pos[0], 2, MPI_FLOAT, workingPid, 115, MPI_COMM_WORLD); //tell tracker of birth - send childid, x,y
//			printf("S %d after sending pos to child %d \n", rank, workingPid);
		}
	}
}

int Squirrel::willSquirrelDie() {
	death = 0;
	if(infected == 0) { //does it catch infection
		infected = willCatchDisease(avgInfLevel, &state);
		if(infected == 1) { //only set infectedStep once
			infectedStep = numSteps;
		}
	}

	if(infected == 1) { //will it die? call every time after initial infection
		if(numSteps - infectedStep > 50) {
			death = willDie(&state);
			if(death == 1){
				MPI_Ssend(&death, 1, MPI_INT, TRACKER_RANK, 777, MPI_COMM_WORLD);
				//infected = 0;
			}
		}
	}
	return death;
}


int Squirrel::getSteps(){
	return numSteps;
}	

int Squirrel::getBirthValue() {
	return birth;
}

int Squirrel:: getDeathValue() {
	return death;
}

int Squirrel::getInfectedValue() {
	return infected;
}	



