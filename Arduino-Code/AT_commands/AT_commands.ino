#include <SoftwareSerial.h>
SoftwareSerial bt(7,8); //rx, tx

char btData, serialData;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  bt.begin(9600);
  Serial.println("Enter AT commands:");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(bt.available() > 0){
    btData = bt.read();
    Serial.print('a');
    Serial.print(btData);
  }
  if(Serial.available() > 0){
    //Serial.println(Serial.available());
    serialData = Serial.read();
    //Serial.print(bt.write(serialData));
    bt.write(serialData);
    //Serial.println(bt.available());
  }
}
