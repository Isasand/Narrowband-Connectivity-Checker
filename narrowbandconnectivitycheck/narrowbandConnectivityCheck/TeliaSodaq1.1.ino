#include <LiquidCrystal.h>
#include <Device.h>

Device device; 

/* Register selector pin -> pin 2
 * Enable pin -> pin 3
 * Data pin 4 -> pin 4A
 * Data pin 5 -> pin 5
 * Data pin 6 ->pin 6
 * Data pin 7 -> pin 7
 */
 
LiquidCrystal lcd(2,3,4,5,6,7);

/* 
 * btn 1 -> pin 12
 * btn 2 -> pin 11
 * btn 3 -> pin 10 
 * btn 4 -> pin 9 
 * back btn -> pin 8
 */
 
int buttons[5] = {12,11,10,9,8};
//last pressed btn
int btn = 0; 

long lastDebounceTime = 0;  
// the debounce time
long debounceDelay = 10;    

//some custom chars for the lcd (for the signalbar)
uint8_t unfilled[8] = {0x1F,0x11,0x11,0x11,0x11,0x11,0x11,0x1F};
uint8_t box[8] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};

void setup() {
  //send the custom chars to lcd
  lcd.createChar(0, unfilled);
  lcd.createChar(1, box);

  //init buttons 
  for(int i = 0; i<5; i++){
    pinMode(buttons[i], INPUT);
  }
  lcd.begin(20,4);
  lcd.print("STARTING..."); 
  device.init(); 

  //render menu
  menu();
} 


void loop() {
  //stay here until a button is pressed
   while(btn == 0){
    btn = waitForInput();
   }

   //run selected state and page 
   device.state.current = runState(btn, device.state);

   //reset button
   btn = 0; 
}


// may need to clean this up later 
int runState(int btn, Device::states &st){
  //if current state is the menu state, all menu buttons correlate with the next state
  if(st.current == ST_MENU && btn != BACK){
    st.current = btn;
  }
  //pressing back button always results in going back to the first state
  if(btn == BACK){
    goBack();
    //TODO fix so we jut go back one page not to menu everytime
    return ST_MENU;
  }  
  st.page++;
    
  if(st.current == ST_MEASURE){
    switch(st.page){
      case 1:
        lcd.clear(); 
        lcd_println("NARROWBAND\nCAT M1"); 
        break;
      case 2:
        lcd.clear();
        btn == 1 ? device.protocol = NB : device.protocol = CAT;
        
        unsigned long interval=500; 
        unsigned long previousMillis=0;
        Device::measurePoint *m; 
        bool looping = true; 
        while(looping){
          unsigned long currentMillis = millis();
          if ((unsigned long)(currentMillis - previousMillis) >= interval) {
            m = device.measure(); 
            printMeasure(m->quality, m->power, device.protocol);
            previousMillis = millis();
          }


          //TODO change this to waitforinput (for debounce delay)
          if(digitalRead(buttons[4]) == HIGH){
            goBack();
            return ST_MENU;
          }

          if(digitalRead(buttons[3]) == HIGH){
            device.addMeasurePoint(m);
            lcd.clear(); 
            lcd.print("Saved!"); 
            delay(1000); 
            lcd.clear();
          }
        
          if(digitalRead(buttons[2]) == HIGH){
            device.send(m); 
            lcd.clear();
            lcd.print("SENT!");
            delay(1000); 
            lcd.clear();
          }
        }
        delete m; 
        break;
      }
    return ST_MEASURE;
  } 
  if(st.current == ST_GPS){
    switch(st.page){
      case 1:
        if(device.lastCoordinates.lat == 0 && device.lastCoordinates.lon == 0){
          lcd.clear();
          lcd.print("NO GPS");
          lcd.setCursor(10,3); 
          lcd.print("UPDATE GPS");
        }
        else{
          lcd.clear();
          lcd.print(String("LAT: ") + device.lastCoordinates.lat);
          lcd.print(String("LON: ") + device.lastCoordinates.lon);
          lcd.print(String("T: ")+ device.lastCoordinates.time); 
        }
        break;
      case 2:
        //TODO add updating GPS with real data 
        lcd.clear(); 
        lcd.print("Searching..."); 
        delay(2000);
        lcd_println("\nLat : 23.323455\nLon: 34.249599\nT: 201812181206");
      break;
    }
    return ST_GPS;
  }
  
  if(st.current == ST_SAVED){
    switch(st.page){
      case 1: 
        if (device.getSizeOfSaved() == 0){
          lcd.clear();
          lcd.print("NO SAVED DATA"); 
        }

        else{
          lcd.clear(); 
          for(int i = 0; i < device.getSizeOfSaved() && i < 4 ; i++){
            lcd.print("P : ");
            lcd.print(device.measureList[i].power); 
            lcd.print(" Q : "); 
            lcd.print(device.measureList[i].quality); 
            lcd.setCursor(0,i); 
          }
        }
        break;
    }
    return ST_SAVED; 
  }
}

int waitForInput(){
  if ( (millis() - lastDebounceTime) > debounceDelay) {
    if(digitalRead(buttons[0]) == HIGH){
      while(digitalRead(buttons[0]) != LOW){}
        ;
        
      lastDebounceTime = millis();
      return 1; 
    }
    if(digitalRead(buttons[1]) == HIGH){
      while(digitalRead(buttons[1]) != LOW){}
        ;
        
     lastDebounceTime = millis();
     return 2; 
    }
    if(digitalRead(buttons[2]) == HIGH){
      while(digitalRead(buttons[2]) != LOW){}
        ;
        
      lastDebounceTime = millis();
      return 3; 
    }
    if(digitalRead(buttons[3]) == HIGH){
      while(digitalRead(buttons[3]) != LOW){}
        ;
        
     lastDebounceTime = millis();
     return 4; 
    }
    if(digitalRead(buttons[4]) == HIGH){
      while(digitalRead(buttons[4]) != LOW){}
        ;
        
     lastDebounceTime = millis();
     return BACK; 
    }
  }
  return 0;
}



void printMeasure(int qual, int power, int protocol){
  lcd.clear(); 
  int i = 1;
  
  int signalbar = qual/2;
  int displaynumbers = qual;
  int lastQ = 0; 
  int lastP = 0; 
  
  lcd.home();  
  
  if(displaynumbers > 254){
      lcd.print("Searching..."); 
  }
  else{
    lcd.print("Q");
    for(int k = 0; k < 2; k++){
      for(i;i <= signalbar; i++){
        lcd.print(char(1)); 
        lcd.setCursor(i, k);
      }
    for(int j = i; j <= 18 ; j++){
      lcd.print(char(0)); 
      lcd.setCursor(j,k);
    }
    lcd.print(displaynumbers); 
    lcd.setCursor(0,1);
    lcd.print("P"); 
    i = 1; 
    signalbar = power/6; 
    displaynumbers = power;
    }
  }
  lcd.setCursor(0,3);
  String p = (device.protocol == 0) ? "LTE NB" : "LTE CAT M1";
  
  lcd.print(p);
  lcd.setCursor(16,2); 
  lcd.print("SEND");
  lcd.setCursor(16,3);
  lcd.print("SAVE");
}
void goBack(){
  menu(); 
  device.state.page = 0;
}

void menu(){
  lcd.clear(); 
  lcd_println("MEASURE\nGPS\nSAVED");
}


void lcd_println(char *str){
  int i = 0; 
  char* c = str; 
  lcd.setCursor(0,0);
  while(*str){
    if(*str == '\n'){
      lcd.setCursor(0, ++i);
    }
    else{
      lcd.print(*str); 
    }
    str++;
  }
}
