#ifndef _SaraSerial_h
#define _SaraSerial_h

class SaraSerial 
{
  	public: 
		SaraSerial(); 
    	void init();
    	static void writeCommand(char* command);
		void readCommand();
		int ackCommand(); 
		void echo();
		static bool bootComplete();
		static bool okResponse();
		char* readToBuf();
		

		//char buf[50];
};

#endif