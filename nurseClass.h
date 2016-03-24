#ifndef NURSE_H
#define NURSE_H

class Nurse
{
	private: 
		int rank; 
		int incomingMessage;
		int simulationRunning;
	public:
		Nurse();
		void run();
		void newSquirrel();
		void birthNewSquirrel();
};


#endif
