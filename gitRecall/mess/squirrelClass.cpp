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


int Squirrel::oneStep(int stop) {
	updateStep();
	getSquirrelCell();	
	squirrelToCell();
	updateValues();
	giveBirth();
	int toDie = willSquirrelDie();
	int toStop = shouldWorkerStop();
	if( toDie==1 || toStop==1 ) { 
		stop = 1;
	}
	return stop;
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
	int parent = getCommandData();
	if( parent == 0 && (rank - NUM_CELLS - NUM_EXTRA_ACTORS <  NUM_INITIAL_INFECTION) ) { //give four random squirrels infection
		infected = 1;
		infectedStep = 0;
	}
	if(parent != 0 ) {
		int parent = getCommandData();
		MPI_Recv(&pos[0], 2, MPI_FLOAT, parent, 115, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		x = pos[0]; 
		y = pos[1];
//		printf("Newborn %d from parent %d. My starting position is x = %f y=%f \n", rank, parent, x,y);
	}
	printf("Squirrel on rank %d. My parent was %d. Initial infection = %d. Starting Poisition: (%f, %f) \n", rank, parent, infected,x,y);
	for(int i = 0; i<SQUIRREL_NUM_STEPS_RECALLED; i++) {
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
	populationRecentCells[numSteps % SQUIRREL_NUM_STEPS_RECALLED ] = populationCurrentCell;
	infectionRecentCells[numSteps % SQUIRREL_NUM_STEPS_RECALLED] = infectionCurrentCell;
//	MPI_Recv(&cellValues[0], 2, MPI_INT, cellRank, 112, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
//	populationRecentCells[numSteps % 50 ] = cellValues[0];
//	infectionRecentCells[numSteps % 50] = cellValues[1];
		
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


int Squirrel::getSimulation() {
	return simulationRunning;
}

void Squirrel::updateValues() {
	avgInfLevel = 0;
	avgPopInflux = 0;
	for(int i = 0; i < SQUIRREL_NUM_STEPS_RECALLED; i++) { //could be done more efficiently
		avgInfLevel += infectionRecentCells[i];
		avgPopInflux += populationRecentCells[i];
	}
	avgInfLevel = avgInfLevel/SQUIRREL_NUM_STEPS_RECALLED;
	avgPopInflux = avgPopInflux/SQUIRREL_NUM_STEPS_RECALLED;
}

void Squirrel::giveBirth() {
	int workingPid;
	birth = 0;
	if(numSteps % BIRTH_CHECK_FREQ == 0) {
		birth = willGiveBirth(avgPopInflux, &state);
		if(birth == 1 && shouldWorkerStop() == 0) {
			workingPid = startWorkerProcess();
			MPI_Ssend(&workingPid, 1, MPI_INT, TRACKER_RANK, 778, MPI_COMM_WORLD); //tell tracker of birth - send childid, x,y
			MPI_Ssend(&pos[0], 2, MPI_FLOAT, workingPid, 115, MPI_COMM_WORLD); //tell tracker of birth - send childid, x,y
		}
	}
}

int Squirrel::willSquirrelDie() {
	death = 0;
	if(infected == 0) { //does it catch infection
		if(willCatchDisease(avgInfLevel, &state)==1){
			printf("S %d catching infection at step %d \n", rank, numSteps);
			infected = 1;
			infectedStep = numSteps;
			MPI_Ssend(&infected, 1, MPI_INT, TRACKER_RANK, 776, MPI_COMM_WORLD);
		}	
	}
	if(infected == 1) { //will it die? call every time after initial infection
		if(numSteps - infectedStep > 50) {
			death = willDie(&state);
			if(death == 1){
				MPI_Ssend(&death, 1, MPI_INT, TRACKER_RANK, 777, MPI_COMM_WORLD);
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




