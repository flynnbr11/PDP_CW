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


int main(int argc, char* argv[]) {

	MPI_Init(&argc, &argv);
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
	MPI_Finalize();
	if(tempRank == 0) {
		printf("AT END OF SIMULATION \n");
	}
	return 0;
}


