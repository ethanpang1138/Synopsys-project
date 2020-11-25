#include <SoftwareSerial.h>

char command;
String string;
boolean ledon = false;
#define led 11
SoftwareSerial bt(7,8); //(rx, tx)

  void setup()
  {
    Serial.begin(9600);
    bt.begin(9600);
    pinMode(led, OUTPUT);
  }

  void loop()
  {
    if (bt.available() > 0) 
    {string = "";}
    
    while(bt.available() > 0)
    {
      command = ((byte)bt.read());
      
      if(command == ':')
      {
        break;
      }
      
      else
      {
        string += command;
      }
      
      delay(1);
    }
    
    if(string == "TO")
    {
        ledOn();
        ledon = true;
    }
    
    if(string =="TF")
    {
        ledOff();
        ledon = false;
        Serial.println(string);
    }
    
    if ((string.toInt()>=0)&&(string.toInt()<=255))
    {
      if (ledon==true)
      {
        analogWrite(led, string.toInt());
        Serial.println(string);
        delay(10);
      }
    }
 }
 
void ledOn()
   {
      analogWrite(led, 255);
      delay(10);
    }
 
 void ledOff()
 {
      analogWrite(led, 0);
      delay(10);
 }
 

    
