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
#include "actorFunctions.h"
using namespace std;
/*
* The main function calls MPI_Init
* 	- Rank 0 assigned as master
* 	- All other ranks become other actors
* 	- Actor functions defined in actorFunctions.h 
*/

int main(int argc, char* argv[]) {
	double start_time, end_time, total_time;
	MPI_Init(&argc, &argv);
	start_time = MPI_Wtime();
	int statusCode = processPoolInit();
	int tempRank; 
	MPI_Comm_rank(MPI_COMM_WORLD, &tempRank);

	if (statusCode == 1) { //Workers have statusCode==1
		actorCode();
	}

	else if (statusCode == 2) { //Master has statusCode==2
		masterCode();
		printf("Master finishing\n");
	}		

	processPoolFinalise();
	end_time = MPI_Wtime();
	MPI_Finalize();

	if(tempRank == 0) {
		total_time = end_time - start_time;
		printf("AT END OF SIMULATION \n Time = %f seconds \n", total_time);
	}
	return 0;
}


