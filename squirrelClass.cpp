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
//	cellRank = 1;
	numSteps = 0;
	flag = 0;
	tag = -1;
	death = 0;
	birth = -1;
	eligibleToDie = 0;
	if(rank % 4 ==0) infected = 1;
	simulationRunning = 1;
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
	int incomingMessage=-1;
	MPI_Send(&infected, 1, MPI_INT, cellRank, 111, MPI_COMM_WORLD);
	MPI_Recv(&infectionCurrentCell, 1, MPI_INT, cellRank, 112, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
	MPI_Recv(&populationCurrentCell, 1, MPI_INT, cellRank, 113, MPI_COMM_WORLD, MPI_STATUS_IGNORE);		
	populationRecentCells[numSteps % 50 ] = populationCurrentCell;
	infectionRecentCells[numSteps % 50] = infectionCurrentCell;
		
	//printf("Squirrel %d is on cell %d with pop %d and inf %d \n", rank, cellRank, populationCurrentCell, infectionCurrentCell);
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
	//		printf("Received poison S %d \n", rank);
			simulationRunning = 0;
		}
		else printf("probe gives Tag = %d \n", tag);
		flag = 0;	
	} 
}


void Squirrel::updateStep() {
	float x_new, y_new;
	squirrelStep(x,y,&x_new,&y_new,&state); //initilise squirrel positions	
	//printf("update before: S%d x =%f, y=%f x_new=%f, y_new=%f\n",rank, x,y, x_new, y_new); 
	x=x_new;
	y=y_new;	
	numSteps ++;
	////printf("update after: S%d x =%f, y=%f x_new=%f, y_new=%f\n",rank, x,y, x_new, y_new); 
}


// receive function deprecated
void Squirrel::receiveInfectionLevel() {
	MPI_Recv(&infectionCurrentCell, 1, MPI_INT, cellRank, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Recv(&populationCurrentCell, 1, MPI_INT, cellRank, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	populationRecentCells[numSteps % 50 ] = populationCurrentCell;
	infectionRecentCells[numSteps % 50] = infectionCurrentCell;
	//printf("Squirrel %d on cell %d which has infection %d and population %d \n", rank, cellRank, infectionCurrentCell, populationCurrentCell);
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
	float pos[2];
	birth = willGiveBirth(avgPopInflux, &state);
	if(birth == 1) {
		pos[0] = x;
		pos[1] = y;
		//printf("S %d birthing: p0 = %f p1=%f x=%f y=%f \n", rank, pos[0], pos[1], x,y);
		MPI_Send(&pos[0], 2, MPI_FLOAT, NURSE_RANK, 222, MPI_COMM_WORLD);
		
		//printf("squirrel %d Giving birth\n", rank);		
	}


}

void Squirrel::willSquirrelDie() {
	if(infected == 0) { //does it catch infection
		infected = willCatchDisease(avgInfLevel, &state);
		if(infected == 1) { //only set infectedStep once
			infectedStep = numSteps;
			//printf("S %d has caught inf at step %d \n", rank, infectedStep);
		}
	}

	if(infected == 1) { //will it die? call every time after initial infection
		if(numSteps - infectedStep > 50) {
			death = willDie(&state);
			if(death == 1) {
				MPI_Send(&death, 1, MPI_INT, 2, 777, MPI_COMM_WORLD);
				//printf("S %d sent death bell to tracker at step %d \n", rank, numSteps);
			}	
		}
	}
	
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



