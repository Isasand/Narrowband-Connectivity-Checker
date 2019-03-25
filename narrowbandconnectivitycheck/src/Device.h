#ifndef _DEVICE_H_
#define _DEVICE_H_

#define MAX_POINTS 5
#include <SodaqSerial.h>
#include <Sodaq_UBlox_GPS.h>

/*
 * To use Sodaq_UBlox_GPS.h>
 * you need to include <Stream.h> in their header file
 */

#include <WString.h>
#include <Arduino.h>

#define ST_MEASURE 1
#define ST_GPS 2
#define ST_SAVED 3
#define ST_MENU 0
#define BACK 9
#define NB 0
#define CAT 1

class Device{
    
public:
    
    struct measurePoint
    {
        int power;
        int quality;
        
    };
    
    struct states{
        int current = ST_MENU;
        int page = 0;
    } state;
    
    struct coordinates
    {
        float lat;
        float lon;
        String time;
        int sat;
    } lastCoordinates;
    
    Device();
    void init();
    void send(measurePoint *point);
    void browseSaved();
    void updateGPS();
    void addMeasurePoint(measurePoint*);
    int getSizeOfSaved(); 
    measurePoint *measure();
    coordinates getLastCoordinates();
    
    bool protocol;
    measurePoint measureList[MAX_POINTS];
    SodaqSerial serial;
    String IMSI = "238208700440116";
    
private:
    void getP_Q(char* str , measurePoint*);
    int scan_GPS(uint32_t delay_until);
    uint8_t sizeOfSaved;
    bool noSocket = true;
};

#endif
