#include <SodaqSerial.h>
#include <Arduino.h>
#include <string.h>
#define UBLOX Serial1
#define DEBUG_SERIAL SerialUSB
#define BAUD 115200


SodaqSerial::SodaqSerial(){
  
}

void SodaqSerial::init(){
	//start module
	pinMode(SARA_ENABLE, OUTPUT);
	pinMode(SARA_TX_ENABLE, OUTPUT);
  	pinMode(SARA_R4XX_TOGGLE, OUTPUT);
  
  	digitalWrite(SARA_ENABLE, HIGH);
  	digitalWrite(SARA_TX_ENABLE, HIGH);
  	digitalWrite(SARA_R4XX_TOGGLE, LOW);
  	
	//start communication
	UBLOX.begin(BAUD);
  	DEBUG_SERIAL.begin(BAUD);
	
	//while(!DEBUG_SERIAL){}
		;

	DEBUG_SERIAL.print("Starting...");
	
	while(!bootComplete()){}
		; 
	
	DEBUG_SERIAL.println("Booting complete.");
}

void SodaqSerial::writeCommand(char* command){
	char *ptr = command; 
	UBLOX.write(ptr);  
}

void SodaqSerial::readCommand(){
	while(!UBLOX.available()){}
		;
	delay(200);
	while(UBLOX.available()){
		DEBUG_SERIAL.write(UBLOX.read()); 
	}
}

char *SodaqSerial::readToBuf(){
	//TODO Remove daley, add OK control
	uint8_t count = 0;
	while(!UBLOX.available()){}
		;
	delay(200);
	while(UBLOX.available()){
		char a = UBLOX.read();
		buf[count++] = a;
	}
	return buf;
}

int SodaqSerial::ackCommand(){
	char buf[255];
	int count = 0; 
	while(!UBLOX.available()){
	};
	delay(200);
	while(UBLOX.available()){
		buf[count++] = UBLOX.read();
	}
	buf[count] = '\0';
	if(strstr(buf, "OK\r\n") != NULL){
		DEBUG_SERIAL.println("OK!");
		return 0; 
	}
	else if(strstr(buf, "ERROR\r\n") != NULL){
		DEBUG_SERIAL.println("ERROR!");  
	}
	return 1;  
}

bool SodaqSerial::bootComplete(){
	bool r = false;
	SodaqSerial::writeCommand("AT\r\n");
	(UBLOX.find("OK\r\n")) ? r = true : r = false;
	DEBUG_SERIAL.print(".");
	return r;
}

void SodaqSerial::echo(){	
	while(DEBUG_SERIAL.available()){
		uint8_t c = DEBUG_SERIAL.read(); 
		UBLOX.write(c); 
	}
	while(UBLOX.available()){
		DEBUG_SERIAL.write(UBLOX.read()); 
	}  
}

void SodaqSerial::startRoutine(){

}
