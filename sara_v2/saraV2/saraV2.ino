/*
OLED -> Sodaq SARA
VCC -> 3V
GND -> GND
SDA -> SDA 
SCL -> SCL 

Bluetooth -> Sodaq SARA 
VCC -> 3.3V
GND -> GND
TX -> RX
RX -> TX
STATE -> D7
*/

#include <device.h>
#include <saraSerial.h>
#include <Arduino.h>
#undef max 
#undef min
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <string.h>
//#include <string>
#include <stdlib.h>

#define DISPLAY_ADR 0x3C
#define TEXT_START_CMD "ALERT:"
#define STOP_CMD '#'
#define ECHO_START_CMD "ECHO:"
#define BT Serial
#define ADC_AREF 3.3f
#define BATVOLT_R1 4.7f // SFF, AFF, One v2 and v3
#define BATVOLT_R2 10.0f // SFF, AFF, One v2 and v3
#define BATVOLT_PIN BAT_VOLT

Adafruit_SSD1306 display(-1);

Device device;
Device::measurePoint mp;
bool refreshed = false;
bool toggled = false;
int prev_power = 0; 
int prev_quality = 0;
String qp;


void setup(){

  device.init();
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  
  printStartupScreen();
  device.getURAT();
  delay(1000);
  
  if(device.protocol){
    device.toggleProtocol();
  }
  
  device.getIMSI();
  printOLED("Waiting for\nbluetooth connection");
  
  while(!bluetoothConnected())
  {
    ;
  }
  printOLED("Connected");
  battOLED();
}

void loop() {
  
 while(bluetoothConnected()){
    
    char c = '\0';

    while (!BT.available() && bluetoothConnected())
    {
      ;
    }
    
    while (BT.available())  
    {
      c = BT.read();  //gets one byte from serial buffer
      delay(5);
      break;       
    }
  
    switch(c) {

    case '0': //Send IMSI and Protocol.

        sendIMSIandProtocol();
      
        break;
     
     case '3': //NB refresh

        refreshed = device.refresh();
        
        if(refreshed)
        {
          printOLED("NB1 detach &\nattach performed with\nsuccess.");
          BT.print(TEXT_START_CMD);
          BT.print("Refresh succesful");
          BT.print(STOP_CMD);
          BT.flush();
        }
        else
        {
          printOLED("No success.");
          BT.print(TEXT_START_CMD);
          BT.print("Refresh not succesful");
          BT.print(STOP_CMD);

        }
          //no break -> continue to next case to Get Q&P.
  
      case '1': //Get Signal Power and Quality.
          
          while (!BT.available() && bluetoothConnected())
          {
            
            mp = device.measure();
             
            if(prev_power != mp.power && prev_quality != mp.quality)
            {

              BT.print("PQ:");
              BT.print(mp.quality);
              BT.print(':');
              BT.print(mp.power);
              BT.print(STOP_CMD);
              BT.flush();
              
              prev_power = mp.power;
              prev_quality = mp.quality;
              
              qp = "Q: " + String(mp.quality) + "/34\nP: " + String(mp.power) + "/97";
              p_qOLED(qp); 
              
             }
          }
        
        break;
  
      case '2': //Toggle between NB1 & M1
      
        toggled = device.toggleProtocol();

        if(toggled)
        {
            printOLED("Toggle performed with\nsuccess.");
            BT.print(TEXT_START_CMD);
            BT.print("Toggle succesful");
            BT.print(STOP_CMD);  
        }
        else
        {
            BT.print(TEXT_START_CMD);
            BT.print("Toggle not succesful");
            BT.print(STOP_CMD);          
        }

        device.getURAT();
        BT.print("PROTOCOL:");
        BT.print(device.protocol);
        BT.print(STOP_CMD);
        BT.flush();

        prev_power = 0; 
        prev_quality = 0;
        
        break;
        
      case '4': //Attach (AT+CFUN=1)
        if(device.attach()){
          BT.print(TEXT_START_CMD);
          BT.print("Attach successful");
          BT.print(STOP_CMD);
          BT.flush();
          prev_power = 0;
          prev_quality = 0;
          printOLED("Attach successful");
        }
        else
        {
          BT.print(TEXT_START_CMD);
          BT.print("A_FALSE");
          BT.print(STOP_CMD);
          BT.flush();
          prev_power = 0;
          prev_quality = 0;
          printOLED("Attach failed");
        }
        break;
  
      case '5': //Detach (AT+CFUN=0)
        if(device.detach())
        {
          BT.print(TEXT_START_CMD);
          BT.print("Detach successful");
          BT.print(STOP_CMD);
          BT.flush();
          prev_power = 0;
          prev_quality = 0;
          printOLED("Detach successful");          
        }
        else
        {
          BT.print(TEXT_START_CMD);
          BT.print("D_FALSE");
          BT.print(STOP_CMD);
          BT.flush();
          prev_power = 0;
          prev_quality = 0;
          printOLED("Detash failed");
        }
        break;
      
      case '6':{ //send to UBLOX echo server

        char data[255];
        char ip[50];
        char port[10];
        int datalength;
        int count = 0;
        
        while(BT.available())
        {
          data[count++] = BT.read();
          
        }
        data[count] = '\0';
        datalength = strlen(data);
        
        char dataSize[255];
        itoa(datalength, dataSize, 10);

        device.sendToServer(data, dataSize, "195.34.89.241", "7"); //Values for sending to UBLOX echo server.
        //delay(1000);
        char* answer = device.readSocket(dataSize);
        printOLED(answer);
        BT.print(ECHO_START_CMD);
        BT.print(answer);
        BT.print(STOP_CMD);
        BT.flush();
        
      }
        break;
        
      default:
        printOLED("No matching function");
        break;
    }
  }
  
  printOLED("Bluetooth not\nconnected");
  prev_power = 0; 
  prev_quality = 0;
 

}

  void printOLED(String str)
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.clearDisplay();
    display.setCursor(0,0);
    display.print(str);
    display.display();
    delay(2000);
    display.clearDisplay();
    display.display();
  }

  void p_qOLED(String str){
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.clearDisplay();
    display.setCursor(0,0);
    display.print(str);
    display.display();
  
  }

  void printStartupScreen(){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,4);
    display.println("   Telia Company\n     NB & LTE-M");
    display.println("Connectivity Checker");
    display.display(); // update display with all of the above graphics
    delay(1500);  
    display.clearDisplay();
    display.display();
  }

  void batteryOLED(uint16_t str)
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Battery: ");
    display.print(str);
    display.print(" mVolt");
    display.display();
    delay(2000);
    display.clearDisplay();
    display.display();
  }

  void battOLED(){

    uint16_t batt = getBatteryVoltage();
    batteryOLED(batt);
    
  }

  void sendIMSIandProtocol()
  {
    BT.print("PROTOCOL:");
    BT.print(device.protocol);
    BT.print(STOP_CMD);
    BT.print("IMSI: " + device.IMSI + STOP_CMD);
    BT.flush(); //flush needs for app to have time to catch this.
  }

  uint16_t getBatteryVoltage()
  {
    uint16_t voltage = (uint16_t)((ADC_AREF / 1.023) * (BATVOLT_R1 + BATVOLT_R2) / BATVOLT_R2 * (float)analogRead(BATVOLT_PIN));

    return voltage;
  }

  bool bluetoothConnected() //works with first gen bluetooth device with state pin sending ones and zeros.
  {
    int readPin = 0;
    bool looping = true;
    int count = 0;
    int pinState = 0;
           
    while(looping)
    {
      pinState = digitalRead(7);
      if(pinState == 1)
      {
        readPin++;
        count++;
      }
      else
      {
        readPin--;
        count++;
      }

      if(count == 100000)
      {
        if(readPin < 100000)
        {
          readPin = 0; 
          return false;
        }
        else
        {  
          readPin = 0;
          return true;
        }
      }
    } 
  }
