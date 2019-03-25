#include <SodaqSerial.h>
#include <string.h> 

SodaqSerial sara;
 
void setup() {
  SerialUSB.begin(115200);
  sara.init();  
  startRoutine(); 
}

void loop() {
  sara.echo(); 
  
}

void startRoutine(){
  char commands[] = "AT+COPS\r\n,AT+dd\r\n"; 
  char* ptr = strtok(commands, ",");
  
  while(ptr!= NULL){
    sara.writeCommand(ptr);
    sara.ackCommand(); 
    ptr = strtok(NULL, ","); 
  }
  SerialUSB.println("finished");
  
}
