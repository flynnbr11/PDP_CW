#include <stdio.h>
#include "mpi.h"
#include "pool.h"
#include "squirrel-functions.h"
#include "ran2.h"
#include <iostream>
#include "parameters.h" 

using namespace std;
#include "squirrelClass.h"

/*
* Squirrel class. 
* Methods to perform all operations required throughout lifetime. 
*/


/*
* Constructor assigns rank within communicator to rank
*/
Squirrel::Squirrel(){
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	state = - 1 - rank;
	initialiseRNG(&state);
}

/*
* All actions of one move by squirrel. Calls other methods within class. 
* Returns 1 if the squirrel is determined to die, or shutDownPool has been called
* Returns 0 otherwise
*/
int Squirrel::oneStep(int stop) {
	updateStep();
	getSquirrelCell();	
	squirrelToCell();
	updateValues();
	giveBirth();
	int toDie = willSquirrelDie();
	// Find out if shutDownPool has been called
	int toStop = shouldWorkerStop();
	if( toDie==1 || toStop==1 ) { 
		stop = 1;
	}
	return stop;
}


/*
* Initialise values to be used throughout lifetime of squirrel actor. 
* 	- Infect the first NUM_INITIAL_INFECTION squirrels 
* 	- If born by parent squirrel instead of master, wait to be told initial x,y
*/
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
	}
	/*
	* Print below statement to see all births and initial positions.
	* Use to check that ranks are reused after death and positioning of newborns correct.
	*/
	// printf("Squirrel on rank %d. My parent was %d. Initial infection = %d. Starting Poisition: (%f, %f) \n", rank, parent, infected,x,y);
	for(int i = 0; i<SQUIRREL_NUM_STEPS_RECALLED; i++) {
		infectionRecentCells[i]=0;
		populationRecentCells[i]=0;
	}

}

int Squirrel::getRank() {
	return rank;
}	

/*
* Find out cell the squirrel has landed on.
* getCellFromPosition() returns an ID between 1 and 16
* Cells are on hard-coded to start after special actor types, so account for 
* 	that discrepancy here. 
*/ 
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
}

/*
* Find out where squirrel moves to during this step.
*/
void Squirrel::updateStep() {
	float x_new, y_new;
	
	//initilise squirrel positions	
	squirrelStep(x,y,&x_new,&y_new,&state); 
	x=x_new;
	y=y_new;	
	numSteps ++;
	pos[0] = x;
	pos[1] = y;
}


void Squirrel::updateValues() {
	avgInfLevel = 0;
	avgPopInflux = 0;
	for(int i = 0; i < SQUIRREL_NUM_STEPS_RECALLED; i++) { 
		avgInfLevel += infectionRecentCells[i];
		avgPopInflux += populationRecentCells[i];
	}
	avgInfLevel = avgInfLevel/SQUIRREL_NUM_STEPS_RECALLED;
	avgPopInflux = avgPopInflux/SQUIRREL_NUM_STEPS_RECALLED;
}

/*
* Every fiftieth step, determine whether to give birth
* 	- If so, inform tracker, send position to new born. 
*/

void Squirrel::giveBirth() {
	int workingPid;
	birth = 0;
	if(numSteps % BIRTH_CHECK_FREQ == 0) {
		birth = willGiveBirth(avgPopInflux, &state);
		if(birth == 1 && shouldWorkerStop() == 0) {
			workingPid = startWorkerProcess();
			MPI_Ssend(&workingPid, 1, MPI_INT, TRACKER_RANK, 778, MPI_COMM_WORLD); 
			MPI_Ssend(&pos[0], 2, MPI_FLOAT, workingPid, 115, MPI_COMM_WORLD); 
		}
	}
}

/*
* If uninfected
* 	- Determine whether to catch infection
*		- If so, inform tracker	
*
* If infected for over fifty steps
* 	- Determines whether to die
*
* If going to die
* 	- Inform tracker
*
*	Return 1 if squirrel will die; 0 if not. 
*/
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





