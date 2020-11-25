#include <SoftwareSerial.h> 
#define RELAY 10 
#define LIGHT 11 
SoftwareSerial btm(0,1); // rx tx 
int index = 0; 
char data[10]; 
char c; 
boolean flag = false;
void setup() { 
 pinMode(RELAY,OUTPUT); 
 pinMode(LIGHT,OUTPUT); 
 digitalWrite(RELAY,HIGH); 
 digitalWrite(LIGHT,LOW); 
 btm.begin(9600); 
} 
void loop() { 
   if(btm.available() > 0){ 
     while(btm.available() > 0){ 
          c = btm.read(); 
          delay(10); //Delay required 
          data[index] = c; 
          index++; 
     } 
     data[index] = '\0'; 
     flag = true;   
   }  
   if(flag){ 
     processCommand(); 
     flag = false; 
     index = 0; 
     data[0] = '\0'; 
   } 
} 
void processCommand(){ 
 char command = data[0]; 
 char inst = data[1]; 
 switch(command){ 
   case 'R': 
         if(inst == 'Y'){ 
           digitalWrite(RELAY,LOW); 
           btm.println("Relay: ON"); 
         } 
         else if(inst == 'N'){ 
           digitalWrite(RELAY,HIGH); 
           btm.println("Relay: OFF"); 
         } 
   break; 
   case 'L': 
         if(inst == 'Y'){ 
           digitalWrite(LIGHT,HIGH); 
           btm.println("Light: ON"); 
         } 
         else if(inst == 'N'){ 
           digitalWrite(LIGHT,LOW); 
           btm.println("Light: OFF"); 
         } 
   break; 
 } 
} 
