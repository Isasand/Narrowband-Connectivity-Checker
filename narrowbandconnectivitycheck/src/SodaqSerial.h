#ifndef _SodaqSerial_h
#define _SodaqSerial_h

class SodaqSerial 
{
  	public: 
		SodaqSerial(); 
    	void init();
    	static void writeCommand(char* command);
		void readCommand();
		int ackCommand(); 
		void echo();
		static bool bootComplete();
		void startRoutine();
		char* readToBuf();

		char buf[50];
};

#endif
