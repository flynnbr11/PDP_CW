#include <stdio.h>
#include "mpi.h"
#include "pool.h"
#include "squirrel-functions.h"
#include "ran2.h"
#include <iostream>
#include "cellClass.h"
#include "squirrelClass.h"
#include "clockClass.h"
#include "trackerClass.h"
#include "parameters.h" 
#include "unistd.h"
using namespace std;


static void workerCode();
static void masterCode();
void squirrelCode();
void gridCode();
void clockTime();
void receiveLoop(Cell gridPoint);
void trackerCode();


int main(int argc, char* argv[]) {

	MPI_Init(&argc, &argv);
	int statusCode = processPoolInit();
	int tempRank; 
	MPI_Comm_rank(MPI_COMM_WORLD, &tempRank);

	if (statusCode == 1) { //Workers have statusCode==1
		workerCode();
	}

	else if (statusCode == 2) { //Master has statusCode==2
		masterCode();
		printf("Master finishing\n");
	}		
	processPoolFinalise();
	MPI_Finalize();
	if(tempRank == 0) {
		printf("AT END OF SIMULATION \n");
	}
	return 0;
}

static void masterCode() {
	int myRank, parentId;
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	for(int j = 0; j < NUM_CELLS + NUM_SQUIRRELS + NUM_EXTRA_ACTORS; j++) {
		int workerPid = startWorkerProcess();
	}
	
	do{ 
		
	} while(masterPoll());

}	


static void workerCode() {
		int myRank, parentId;
		MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
		

		if(myRank > NUM_EXTRA_ACTORS  && myRank <= NUM_EXTRA_ACTORS  + NUM_CELLS) {
			gridCode();
			printf("Cell %d finishing\n", myRank);
		}	
		
		else if(myRank > NUM_CELLS + NUM_EXTRA_ACTORS) {
			squirrelCode();
			printf("S %d finishing\n", myRank);
		}
		else if(myRank == CLOCK_RANK) {
			clockTime(); // To keep track of the living squirrels etc
			printf("Clock %d finishing\n", myRank);
		}	
		else if(myRank == TRACKER_RANK) {
			trackerCode();
			printf("Tracker %d finishing\n", myRank);
		}
}


void trackerCode() {
	Tracker track;
	track.counter();
}



void clockTime() {
	Clock masterProcess;
	masterProcess.timer();
}

void gridCode() {
	Cell gridPoint;
	gridPoint.run();	
	gridPoint.receiveInfo();
}


void squirrelCode(){
	Squirrel initial;
	int continueSim=1;
	int stop = 0;
	int willDie;
	int myRank = initial.getRank();
	initial.run();
	while (continueSim == 1 ) {
		stop = 0;
		while(stop==0) {
			if(shouldWorkerStop () == 1) {
				break; //break out of while(stop)
				stop = 1;
			}
			initial.updateStep();
			initial.getSquirrelCell();	
			initial.squirrelToCell();
			initial.updateValues();
			initial.giveBirth();

			if(initial.willSquirrelDie() || shouldWorkerStop() ) { 
				stop = 1;
			}
		}
			
		continueSim = workerSleep();
	
		if(continueSim == 0){
			MPI_Ssend(&myRank, 1, MPI_INT, TRACKER_RANK, 773, MPI_COMM_WORLD);
		}
	}
}
