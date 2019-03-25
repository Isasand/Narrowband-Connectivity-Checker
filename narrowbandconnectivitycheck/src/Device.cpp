#include <Device.h>

Device::Device(){
    
    
}

void Device::init(){
	serial.init();
	sodaq_gps.init(GPS_ENABLE);
}

Device::measurePoint *Device::measure()
{
	Device::measurePoint *m = new measurePoint();
  	char *response;
	serial.writeCommand("AT+CESQ\r\n");
	response = serial.readToBuf();
  	getP_Q(response, m);
  	return m;
}

Device::coordinates Device::getLastCoordinates()
{
	return lastCoordinates;
}

void Device::send(measurePoint *point)
{
    if(noSocket){
        serial.writeCommand("AT+USOCR=17\r\n");
        serial.ackCommand();
        noSocket = false;
    }
    
    char *msg = "{'IMSI':'238208700440116','P':65,'Q':42}" ;
    
    char *command = "at+usost=0,\"85.226.170.66\",8080,40,\"";
    //+msg +"\"\r\n";
    serial.writeCommand(command);
    serial.writeCommand(msg);
    serial.writeCommand("\"\r\n");
    serial.ackCommand();
    
}

void Device::updateGPS()
{
	uint32_t wait_ms = 1UL * 60 * 100;
	while(!Device::scan_GPS(wait_ms)){}
		; 
}

int Device::scan_GPS(uint32_t delay_until)
{
	int start = millis(); 
	int timeout = 900 * 100; 
	
	if(sodaq_gps.scan(true, timeout)) {
		lastCoordinates.lat = sodaq_gps.getLat(); 
		lastCoordinates.lon = sodaq_gps.getLon(); 
		lastCoordinates.time = sodaq_gps.getDateTimeString(); 
		lastCoordinates.sat = sodaq_gps.getNumberOfSatellites(); 
		return 0;
	}
	else{
		return 1; //no fix
	}
}

void Device::addMeasurePoint(measurePoint *point)
{
  //TODO dynamisk kontroll för ant platser som är upptagna.
  if(sizeOfSaved >= MAX_POINTS)
  {
    for(int i = 0; i<MAX_POINTS; i++){
        measureList[i] = measureList[i+1];
    }
    measureList[MAX_POINTS-1] = *point;
    sizeOfSaved = 5;
  }
  else 
  {
    measureList[sizeOfSaved] = *point;
    sizeOfSaved++;
  }
}


void Device::getP_Q(char* str, measurePoint *mp)
{
  int arr[30];  
  char* ptr = 0;
  int arrSize = 0;
  uint8_t noNet = 255;
  uint8_t zero = 0x2F;
 
  ptr = strtok(str, ",");
  while(ptr != NULL){
    uint8_t a = atoi(ptr);
    arr[arrSize++] = a;
    ptr = strtok(NULL, ",");
  }
  mp->quality = arr[arrSize-2];
  mp->power = arr[arrSize-1];
}

int Device::getSizeOfSaved(){
    return sizeOfSaved;
}

