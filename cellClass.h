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
		int getRank();
		int getIncomingMessage();
		void sendValues();
		void receiveWhileRunning();
		void printStatus();
		int getInfectionLevel();
		int getPopulationInflux();
		int getSimulation();
		void monthlyUpdates();
};

#endif
