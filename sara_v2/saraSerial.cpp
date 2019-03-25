
#include <Arduino.h>
#include <string.h>
#include <saraSerial.h>

#define UBLOX Serial1
#define BLUETOOTH_SERIAL Serial
#define BAUD 115200


SaraSerial::SaraSerial(){
  
}

void SaraSerial::init(){
	//start module
	pinMode(SARA_ENABLE, OUTPUT);
	pinMode(SARA_TX_ENABLE, OUTPUT);
  	pinMode(SARA_R4XX_TOGGLE, OUTPUT);
  	pinMode(7, INPUT);
  
  	digitalWrite(SARA_ENABLE, HIGH);
  	digitalWrite(SARA_TX_ENABLE, HIGH);
  	digitalWrite(SARA_R4XX_TOGGLE, LOW);
  	
	//start communication
	UBLOX.begin(BAUD);
  	BLUETOOTH_SERIAL.begin(9600);

	while(!BLUETOOTH_SERIAL){}
		;
	
	while(!bootComplete()){}
		; 
	
}

void SaraSerial::writeCommand(char* command)
{
	char *ptr = command; 
	UBLOX.write(ptr);  
}

char *SaraSerial::readToBuf()
{
	//char buf[30];
	char* buf = new char[100];
	uint8_t count = 0;
	while(!UBLOX.available()){}
		;
	delay(200);
	while(UBLOX.available()){
		char a = UBLOX.read();
		buf[count++] = a;
	}
	buf[count] = '\0';
	return buf;
}

int SaraSerial::ackCommand()
{
	char ackBuf[30];
	int count = 0; 
	while(!UBLOX.available()){
	};
	delay(200);
	while(UBLOX.available()){
		ackBuf[count++] = UBLOX.read();
	}
	ackBuf[count] = '\0';

	if(strstr(ackBuf, "OK\r\n") != NULL){
		//SerialUSB.print(ackBuf);
		return 1; 
	}
	else if(strstr(ackBuf, "ERROR\r\n") != NULL){
		//SerialUSB.print("ERROR");
	}
	return 0;  
}

bool SaraSerial::bootComplete(){
	bool r = false;
	SaraSerial::writeCommand("AT\r\n");
	(UBLOX.find("OK\r\n")) ? r = true : r = false;
	//BLUETOOTH_SERIAL.print(".");
	return r;
}

bool SaraSerial::okResponse(){
	bool r = false;
	while(!UBLOX.available()){
	};
	delay(200);
	while(UBLOX.available())
	(UBLOX.find("OK\r\n")) ? r = true : r = false;
	return r;
}



