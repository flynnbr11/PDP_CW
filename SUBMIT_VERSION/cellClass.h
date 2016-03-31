#ifndef CELL_H
#define CELL_H

class Cell
{
	private: 
		int rank; 
		int populationInfluxCurrent;
		int populationInflux;
		int populationInfluxRecentMonths[3];
		int infectionLevelCurrent;
		int infectionLevel;
		int infectionLevelRecentMonths[2];
		int incomingMessage;
		int sender;
		int tag;
		int month;
		int simulationRunning;
	public:
		Cell();
		void run();
		void receiveInfo();
		void informSquirrel(int sender, int tag, MPI_Status status);
		void monthlyUpdates(int sender, int tag, MPI_Status status);
		void receivePoison (int sender, int tag, MPI_Status status);
};

#endif
