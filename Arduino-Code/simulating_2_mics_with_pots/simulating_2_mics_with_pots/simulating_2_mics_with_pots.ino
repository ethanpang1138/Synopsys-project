const int micRightPin = A0;  // potentiometer for right hand mic
const int micLeftPin = A1;  // potentiometer for right hand mic
const int micBackPin = A2;
const int micPin[] = {0,1,2};
int micVals[] = {0,0,0};
unsigned int micRightVal = 0;         // value read from the right hand mic
unsigned int micLeftVal = 0;         // value read from the right hand mic
unsigned int micBackVal = 0;
int ledPin[] = {3,5,6,9,10,11}; //analog output ports for led's (right to left)
double aX, aY, bX, bY, cX, cY, rX, rY, theta = 0.0; // x and y components for the two vectors and the resultant vector , the angle of the resultant
double angleDiff[] = {0,0,0,0,0,0}; // how far the angle of each led is from the sound source
double smallestAngle = 100.0; // holds the current smallest angle difference
int indexOfAngle = 0; // holds the index of the led that is closest to the sound source
double ledAngles[] = {0,60,120,180,240,300}; // set angles that led's are positioned at
int ledBrightness[] = {0,0,0,0,0,0}; // brightness for led
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)

//void getMicVals(){
//  for(int i = 0; i < 3; i++){
//    micVal = analogRead(i); // read the analog in value from pot
//    if (micVal < 1024){ // toss out spurious readings
//      if (micVal > signalMax1){
//        signalMax1 = micRightVal; // save just the max levels
//      } else if (micVal < signalMin1){
//        signalMin1 = micVal; // save just the min levels
//      }
//    }
//    peakToPeak1 = signalMax1 - signalMin1; // max - min = peak-peak amplitude
//    micVals[i] = peakToPeak1;
//  }
//}

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

void loop() {
  micRightVal = analogRead(micRightPin); // read the analog in value from pot
  micLeftVal = analogRead(micLeftPin); // read the analog in value from pot
  micBackVal = analogRead(micBackPin);
  unsigned long startMillis= millis(); // Start of sample window
  unsigned int peakToPeak1 = 0; // peak-to-peak level
  unsigned int peakToPeak2 = 0; // peak-to-peak level
  unsigned int peakToPeak3 = 0; // peak-to-peak level
  unsigned int signalMax1 = 0;
  unsigned int signalMin1 = 1024;
  unsigned int signalMax2 = 0;
  unsigned int signalMin2 = 1024;
  unsigned int signalMax3 = 0;
  unsigned int signalMin3 = 1024;
  // collect data for 50 mS
  while (millis() - startMillis < sampleWindow){
    micRightVal = analogRead(micRightPin); // read the analog in value from pot
    micLeftVal = analogRead(micLeftPin); // read the analog in value from pot
    micBackVal = analogRead(micBackPin); // read the analog in value from pot
  
  
    if (micRightVal < 1024){ // toss out spurious readings
      if (micRightVal > signalMax1){
        signalMax1 = micRightVal; // save just the max levels
      } else if (micRightVal < signalMin1){
        signalMin1 = micRightVal; // save just the min levels
      }
    }
    if (micLeftVal < 1024){ // toss out spurious readings
      if (micLeftVal > signalMax2){
        signalMax2 = micLeftVal; // save just the max levels
      } else if (micLeftVal < signalMin2){
        signalMin2 = micLeftVal; // save just the min levels
      }
    }
    if (micBackVal < 1024){ // toss out spurious readings
      if (micBackVal > signalMax3){
        signalMax3 = micBackVal; // save just the max levels
      } else if (micBackVal < signalMin3){
        signalMin3 = micBackVal; // save just the min levels
      }
    }
  }
  peakToPeak1 = signalMax1 - signalMin1; // max - min = peak-peak amplitude
  peakToPeak2 = signalMax2 - signalMin2; // max - min = peak-peak amplitude
  peakToPeak3 = signalMax3 - signalMin3; // max - min = peak-peak amplitude
  String mic1 = "mic1: ";
  String mic2 = "mic2: ";
  String mic3 = "mic3: ";
  Serial.println(mic1 + peakToPeak1);
  Serial.println(mic2 + peakToPeak2);
  Serial.println(mic3 + peakToPeak3);

  // calculate x and y components of the mic vectors with mic1 at 45 degrees and mic2 at 135 degrees
  aX = peakToPeak1 * cos(PI / 6);
  aY = peakToPeak1 * sin(PI / 6);
  bX = peakToPeak2 * cos(5 * PI / 6);
  bY = peakToPeak2 * sin(5 * PI / 6);
  cX = peakToPeak3 * cos(3 * PI / 2);
  cY = peakToPeak3 * sin(3 * PI / 2);
  // combine components of component vectors to find components of resultant vector
  rX = aX + bX + cX;
  rY = aY + bY + cY;
  // find the angle of the sound source in degrees
  theta = atan2(rY, rX) * 180.0 / PI;
  
  // fill array with differences between angle of the sound source and each led angle
  for(int i = 0; i < 6; i++){
    if((theta - ledAngles[i]) > 0){
      angleDiff[i] = (theta - ledAngles[i]);
    } else {
      angleDiff[i] = -(theta - ledAngles[i]);
    }
  }
  smallestAngle = 100; // reset smallest angle for the next loop
  // find the index of the led with the closest angle to the sound source
  for(int i = 0; i < 6; i++){
    if(angleDiff[i] < smallestAngle){
      smallestAngle = angleDiff[i];
      indexOfAngle = i;
    }
  }
  
  // turn the led with the closest angle to the sound on and turn all others off
  for(int i = 0; i < 6; i++){
    // if both mics are close to 0, turn all lights off
    if(peakToPeak1 + peakToPeak2 + peakToPeak3 < 18){
      analogWrite(ledPin[i], 0);
    } else {
       if(i != indexOfAngle){
        ledBrightness[i] = 0;
       } else {
        ledBrightness[i] = 255;
       }
       analogWrite(ledPin[i], ledBrightness[i]);
    }
    
  }
  
  //print the results to the Serial Monitor:
  String angle = "angle: ";
  String micRight = "right mic: ";
  String micLeft = "left mic: ";
  String index = "index: ";
  String smallAngle = "smallest Angle: ";
  Serial.println(angle + theta);
  Serial.println(' ');
//  Serial.println(micLeft + micLeftVal);
//  Serial.println(ledColors[indexOfAngle] + " should be on");
//  for(int i = 0; i < 4; i++){
//    Serial.println(angleDiff[i]);
//  }
//  Serial.println(index + indexOfAngle);
//  Serial.println(smallAngle + smallestAngle);
  delay(2);
}
