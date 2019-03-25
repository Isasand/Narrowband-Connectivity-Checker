#include <device.h>


Device::Device(){}

void Device::init(){
	saraserial.init();
}

void Device::getURAT() //Getting the radio access technology that the device is connected to.  
{
  
  saraserial.writeCommand("AT+URAT?\r\n");
  char *response;
  response = saraserial.readToBuf();
  SerialUSB.print(response);
  if(strstr(response, "8") != NULL)
  {
    protocol = NB1;
  }
  else if(strstr(response, "7") != NULL)
  {
    protocol = M1;
  }

}

void Device::getIMSI() //Send AT-command (AT+CIMI) and get values (respons: 222107701772423\r\nOK) and send to BT.
{

  Device::measurePoint m;
  saraserial.writeCommand("AT+CIMI\r\n");
  char *response;
  response = saraserial.readToBuf();
  doSplit(response, m);
  //delete m;
}

bool Device::attach()
{
  char * response;
  saraserial.writeCommand("AT+CFUN=1\r\n");
  delay(500); //whait for OK response
  response = saraserial.readToBuf();
  SerialUSB.print(response);
  if(strstr(response, "OK\r\n"))
  {
    return true;
    attached = true;
  }
  else
  {
    return false;
  }  
}

bool Device::detach()
{
  char * response;
  saraserial.writeCommand("AT+CFUN=0\r\n");
  delay(1000); //whait for OK response
  response = saraserial.readToBuf();
  SerialUSB.print(response);
  if(strstr(response, "OK\r\n"))
  {
  	attached = false;
    return true;
    
  }
  else
  {
    return false;
  }
}

Device::measurePoint Device::measure()
{
  if(!attached){
	  attached = attach();
	}
  
  Device::measurePoint m;
  char *response;
  saraserial.writeCommand("AT+CESQ\r\n");
  response = saraserial.readToBuf();
  m = doSplit(response, m);
  delete [] response; //TODO: why does this not take affect?
  return m;
}

Device::measurePoint Device::doSplit(char* str, measurePoint mp)
{   

  int arr[30];  
  char* ptr = 0;
  int arrSize = 0;
  String ar[5];
  
  if(strstr(str, "CESQ") != NULL)
  {
    ptr = strtok(str, ",");
    while(ptr != NULL)
    {

      uint8_t a = atoi(ptr);
      arr[arrSize++] = a;
      ptr = strtok(NULL, ",");

    }

    mp.quality = arr[arrSize-2];
    mp.power = arr[arrSize-1];
    return mp;

  }
  else if(strstr(str, "CIMI") != NULL)
  {
    ptr = strtok(str, "\r\n");

    while(ptr != NULL)
    {

      String a = ptr;
      ar[arrSize++] = a;
      ptr = strtok(NULL, "\r\n");

    }

    IMSI = ar[1];
  }
}

bool Device::refresh()
{
	//Detach and attach from NB cell.
  //CFUN=0 (detach)
  //Wait for detach
  //AT+COPS=0 (Automatic connection. For forced connection to given operator use <AT+CESQ=1,2,"24001"> "24001" is for Telia. Note that forced connection will delay the answer considerably)

  saraserial.writeCommand("AT+CFUN=0\r\n");
  delay(1000);
  
  if(!saraserial.ackCommand())
  {
    return 0;
  }
  else
  {
    saraserial.writeCommand("AT+CGATT?\r\n");
    char *response;
    response = saraserial.readToBuf();

    if(strstr(response, "CGATT: 0") != NULL)
    {
      saraserial.writeCommand("AT+CFUN=1\r\n");
      saraserial.ackCommand();
      return 1;
    }
    else
    {
      return 0;
    }

  }

}

bool Device::toggleProtocol()
{
  //Toggle between NB & LTE-M
  //AT+COPS=2 (deregister)
  //AT+URAT=7 Set radio access technology LTE Cat.M1
  //AT+URAT=8 LTE Cat.NB1
  //check CEREG after attach = 1? should be 1!
  char *response;

  saraserial.writeCommand("AT+COPS=2\r\n");

  if(!saraserial.ackCommand())
  {
    //AT+COPS cmd NOT successful.
    return 0;
  }

  else
  {
    if(protocol == NB1)
    {
      saraserial.writeCommand("AT+URAT=7\r\n");
    }
    else
    {
      saraserial.writeCommand("AT+URAT=8\r\n");
    }
    //delay(1000);

    if(saraserial.ackCommand())
    {
      saraserial.writeCommand("AT+COPS=0\r\n");
      //delay(1000);

      response = saraserial.readToBuf();

      if(strstr(response, "OK\r\n") != NULL){

        if(protocol == NB1)
        {
          protocol = M1;
        }
        else
        {
          protocol = NB1;
        }
        
        saraserial.writeCommand("AT+CFUN=0");
        delay(200);
        saraserial.writeCommand("AT+CFUN=1");

        return 1;
      }
      else if(strstr(response, "no network service") != NULL)
      {
        return 0;
      }
    
    }
    else
    {
      return 0;
    }

  }

}

void Device::sendToServer(char* data, char* length, char* ip="195.34.89.241", char* port="7")
{

  if(noSocket){
    saraserial.writeCommand("AT+USOCR=17\r\n");
    saraserial.ackCommand();
    noSocket = false;
  }

  String comma = ",";
  String apost = "\"";
  String START_CMD = "AT+USOST=0,";  

  String command = START_CMD + apost + ip + apost + comma + port + comma + length + comma + apost + data +apost;
  char* cstr = new char[command.length() + 1];
  strcpy(cstr, command.c_str()); 
  saraserial.writeCommand(cstr);
  saraserial.writeCommand("\r\n");
  delete [] cstr;
  delay(500);

  char* junk = saraserial.readToBuf();

}

char * Device::readSocket(char* dataSize)
{

  saraserial.writeCommand("AT+USORF=0,200");
  //saraserial.writeCommand(dataSize);
  saraserial.writeCommand("\r\n");
  char* str = saraserial.readToBuf();
  return str;
}


