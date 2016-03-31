#ifndef Clock_H
#define Clock_H

class Clock
{
	private: 
		int rank;
		int month;
		int numLivingSquirrels;
		int numInfectedSquirrels;
	public:
		Clock();
		void timer();
		void endMonth();
		void poisonPill();
};

#endif
