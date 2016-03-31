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
		int birth;
		int death;
		int flag;
		int tag;
		float pos[2];
	public: 
		Squirrel(); // constructor 
		void run();
		void getSquirrelCell();
		int getRank();
		int getCellRank();
		void squirrelToCell();
		void updateStep();
		int getSimulation();
		void updateValues();
		void giveBirth();
		int willSquirrelDie();
		int oneStep(int stop);
};

#endif
