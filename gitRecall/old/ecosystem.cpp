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

//	if (statusCode == 1 && tempRank < NUM_EXTRA_ACTORS + NUM_SQUIRRELS + NUM_CELLS) { //Workers have statusCode==1
	if (statusCode == 1) { //Workers have statusCode==1
		workerCode();
	}

	else if (statusCode == 2) { //Master has statusCode==2
		masterCode();
	}		
	processPoolFinalise();
	MPI_Finalize();
	if(tempRank == 0) {
		printf("AT END OF PROGRAM \n");
	}
	return 0;
}

static void masterCode() {
	int myRank, parentId;
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

//	for(int j = 0; j < 1 + NUM_EXTRA_ACTORS + NUM_CELLS + NUM_SQUIRRELS; j++) {
	for(int j = 0; j < NUM_CELLS + NUM_SQUIRRELS + 2; j++) {
		int workerPid = startWorkerProcess();
	}
	printf("master done making threads\n");
	
//*
	do{ 
		
	} while(masterPoll());
//*/

//master can startWorkerProcess and then do whatever based on rank of that
	//clockTime();
}	


static void workerCode() {
		int myRank, parentId;
		MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
		

		if(myRank > NUM_EXTRA_ACTORS  && myRank <= NUM_EXTRA_ACTORS  + NUM_CELLS) {
			//printf("I am cell %d \n", myRank);
			gridCode();
			printf("Cell %d finishing\n", myRank);
		}	
		
//		else if(myRank > NUM_CELLS + NUM_EXTRA_ACTORS && myRank <= NUM_EXTRA_ACTORS  + NUM_CELLS + NUM_SQUIRRELS) {
		else if(myRank > NUM_CELLS + NUM_EXTRA_ACTORS) {
			//printf("I am S %d \n", myRank);
			squirrelCode();
			printf("S %d finishing\n", myRank);
		}
		//*
		else if(myRank == CLOCK_RANK) {
			//printf("I am the clock\n");
			clockTime(); // To keep track of the living squirrels etc
			printf("Clock %d finishing\n", myRank);
		}	
		//*/	
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
	receiveLoop(gridPoint);
}


void receiveLoop(Cell gridPoint) {
	int flag = 0;
	MPI_Status status;
	
	int stop = 0;
	
	while(gridPoint.getSimulation()) {
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		if(flag == 1) {			
			gridPoint.receiveInfo();
			gridPoint.printStatus();
			flag = 0;
		}
//		stop = shouldWorkerStop();
	}
//	printf("Cell %d out of loop \n", gridPoint.getRank());
}



void squirrelCode(){
	Squirrel initial;
	int continueSim=1;
	int stop = 0;
	int willDie;
	int myRank = initial.getRank();
	initial.run();
	while (continueSim == 1 ) {
		printf("S %d at start of continue loop \n", initial.getRank());
		stop = 0;
		while(stop==0) {
			if(shouldWorkerStop () == 1) {
				break; //break out of while(stop)
				stop = 1;
			}
			printf("S %d inside stop loop \n", initial.getRank());
			printf("S %d update step \n", initial.getRank());		
			initial.updateStep();
			printf("S %d getSquirrelCell \n", initial.getRank());		
			initial.getSquirrelCell();	
			printf("S %d squirrelToCell \n", initial.getRank());		
			initial.squirrelToCell();
			printf("S %d updateValues \n", initial.getRank());		
			initial.updateValues();
			printf("S %d giveBirth \n", initial.getRank());		
			if(shouldWorkerStop() == 1) break;
			initial.giveBirth();
			printf("S %d willSquirrelDie \n", initial.getRank());
					
			willDie = initial.willSquirrelDie();	
			if(willDie == 1) {
				stop = 1;
				printf("S %d breaking bc dead \n", initial.getRank());
			}

			printf("birth = %d inf = %d death = %d rank = %d \n", initial.getBirthValue(), initial.getInfectedValue(), willDie, initial.getRank());
			//initial.receive();
			printf("S %d will Die if statement \n", initial.getRank());		
			
			if (willDie != 1) {
				printf("S %d checking whether to stop \n", initial.getRank());
				stop = shouldWorkerStop();
				}
			printf("S %d if stop == 1\n", initial.getRank());		
			if(stop == 1) {
				printf( "stop = 1 on rank %d\n", initial.getRank());
			} 
			printf("At end of stop loop rank %d \n", initial.getRank());
		
			//skipToEnd:
		}
		
		
		printf("rank %d going to sleep \n", initial.getRank());
		continueSim = workerSleep();
		printf("S %d cont = %d stop = %d \n", initial.getRank(), continueSim, stop);
	
		if(continueSim == 0){
			printf("S %d sending shut down to tracker \n", initial.getRank());
			printf("eco mpi_send 1 \n");
			MPI_Ssend(&myRank, 1, MPI_INT, TRACKER_RANK, 773, MPI_COMM_WORLD);
			printf("eco mpi_rec 1 \n");
		}
	}
	//printf("Rank %d closing\n", initial.getRank());
}
