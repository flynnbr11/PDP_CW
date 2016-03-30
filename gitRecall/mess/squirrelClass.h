#ifndef SQUIRREL_H
#define SQUIRREL_H
#include "parameters.h"

class Squirrel
{
	private: 
		long state;
		int rank;
		float x;
		float y;
		int infected;
		int numSteps;
		int infectedStep;
		float avgInfLevel;
		float avgPopInflux;
		int populationRecentCells[SQUIRREL_NUM_STEPS_RECALLED];
		int infectionRecentCells[SQUIRREL_NUM_STEPS_RECALLED];
		int populationCurrentCell;
		int infectionCurrentCell;
		int simulationRunning;
		int cellRank;
		int month;
		int birth;
		int death;
		int eligibleToDie;
		int flag;
		int tag;
		int cellValues[2];
		float pos[2];
	public: 
		Squirrel(); // constructor 
		void run();
		void getSquirrelCell();
		int getRank();
		int getCellRank();
		void squirrelToCell();
		void updateStep();
		void receiveInfectionLevel();
		int getSimulation();
		void squirrelPoison();
		void receive();
		void updateValues();
		void giveBirth();
		int getBirthValue();
		int getDeathValue();
		int getInfectedValue();
		int willSquirrelDie();
		int getSteps();
		int oneStep(int stop);
};

#endif
