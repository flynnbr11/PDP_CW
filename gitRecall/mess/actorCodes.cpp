void masterCode() {
	int myRank, parentId;
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	for(int j = 0; j < NUM_CELLS + NUM_SQUIRRELS + NUM_EXTRA_ACTORS; j++) {
		int workerPid = startWorkerProcess();
	}
	
	do{ 
		
	} while(masterPoll());

}	


void actorCode() {
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
	int continueSim=1;
	int stop;
	while (continueSim == 1 ) {
		Squirrel alvin;
		alvin.run();
		stop = 0;
		while(stop==0) {
			stop=alvin.oneStep(stop);
		}	
		continueSim = workerSleep();
	
//		if(continueSim == 0){
		if(shouldWorkerStop() == 1){
			int myRank = alvin.getRank();
			MPI_Ssend(&myRank, 1, MPI_INT, TRACKER_RANK, 773, MPI_COMM_WORLD);
		}
	}
}

