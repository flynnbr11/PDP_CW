#ifndef TRACKER_H
#define TRACKER_H
#include "parameters.h"

class Tracker
{
private:
	
	int rank;
	int numLivingSquirrels;
	int numInfectedSquirrels;
	long state;
	int squirrelArray[NUM_CELLS + NUM_EXTRA_ACTORS + NUM_SQUIRRELS + MAX_NUM_SQUIRRELS];
	int squirrelSum;
	int continueSim;
	int incomingMessage;	
public:
	Tracker();
	void counter();
	void shutDownMessage(int sender, int tag, MPI_Status status);
	void birthMessage(int sender, int tag, MPI_Status status);
	void deathMessage(int sender, int tag, MPI_Status status);
	void endOfMonth(int sender, int tag, MPI_Status status);
	void squirrelInfectedMessage(int sender, int tag, MPI_Status status);
};

#endif
