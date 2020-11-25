#include <SoftwareSerial.h>

SoftwareSerial btm(7,8); // rx tx

char btData;
char serialData;
int index = 0;

void setup() {
  Serial.begin(9600);
  btm.begin(9600);
  
}

void loop() {
  if(index >= 9){
    index = 1;
  } else {
    index++;
  }
  String m = "00message";
  String msg = m + index;
  btm.println(msg);
  Serial.println(msg);
  delay(500);
  
//  if(Serial.available() > 0){
//    serialData = Serial.read();
//    btm.write(serialData);
//  }
}
