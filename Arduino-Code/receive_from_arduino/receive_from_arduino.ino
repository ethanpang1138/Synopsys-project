#include <SoftwareSerial.h>

char incomingByte;  // incoming data
int  LED = 11;      // LED pin
SoftwareSerial btm(12,13); // rx tx

void setup() {
  Serial.begin(9600);
  btm.begin(9600); // initialization
  pinMode(LED, OUTPUT);
  Serial.println("Press 1 to LED ON or 0 to LED OFF...");
}

void loop() {
  if (btm.available() > 0) {  // if the data came
    incomingByte = btm.read(); // read byte
    if(incomingByte == '0') {
       digitalWrite(LED, LOW);  // if 1, switch LED Off
       btm.println("LED OFF. Press 1 to LED ON!");  // print message
    }
    if(incomingByte == '1') {
       digitalWrite(LED, HIGH); // if 0, switch LED on
       btm.println("LED ON. Press 0 to LED OFF!");
    }
  }
}
