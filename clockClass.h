#ifndef Clock_H
#define Clock_H

class Clock
{
	private: 
		int rank;
		int month;
		int monthOver;
		int poison;
	public:
		Clock();
		void endMonth();
		void timer();
		void poisonPill();
};

#endif
