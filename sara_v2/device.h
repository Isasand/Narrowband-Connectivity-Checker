#ifndef _DEVICE_H_
#define _DEVICE_H_

#define MAX_POINTS 5
#include <saraSerial.h>
#include <WString.h>
#include <Arduino.h>


#define NB1 0
#define M1 1

class Device{
    
public:
    
    struct measurePoint
    {
        /*measurePoint(){
            power = new int;
            quality = new int;
        }
        ~measurePoint(){
            delete power;
            delete quality;
        }
        */

        int power;
        int quality;
        
    };



    Device();
    void init();
    bool refresh();
    bool toggleProtocol(); //change protocol NB1 or M1
    void getIMSI(); //IMSI number to send to application
    void getURAT();
    bool attach();
    bool detach();
    measurePoint measure();
    void sendToServer(char*, char*, char*, char*);
    char* readSocket(char*);

    bool noSocket = true;
    bool protocol;
    bool attached = false;
    SaraSerial saraserial;
    String IMSI;
    
private:
    measurePoint doSplit(char* str , measurePoint);
    
};
#endif
