#include <SoftwareSerial.h>

const int MIC_PINS[] = {0, 1, 2, 3}; // analog pins that mics are connected to
const int NUM_MICS = 4;
SoftwareSerial btm(7,8); // rx tx

const String DIRECTION_ID = "00";
const String NO_SOUND_ID = "01";
const String closeID = "**";

char incomingByte; // incoming data
int micVals[NUM_MICS]; // values read from mics (0 to 1023)
int signalMaxs[NUM_MICS]; // temporary max signals from mics
int signalMins[] = {1024, 1024, 1024, 1024}; // temporary min signals from mics
double xComponents[NUM_MICS]; // x components of mic vectors
double yComponents[NUM_MICS]; // y components of mic vectors
const int LED_PINS[] = {3, 5, 6, 7, 8, 9, 10, 11}; //analog output ports for led's (right to left)
const int NUM_LEDS = 8; // number of leds being used
double rX, rY = 0.0; // x and y components for the two vectors and the resultant vector , the angle of the resultant
double theta = 0.0; // angle of the sound source
double theta2 = 0.0;
double angleDiff[NUM_LEDS]; // how far the angle of each led is from the sound source
double smallestAngle = 361.0; // holds the current smallest angle difference
int indexOfAngle = 0; // holds the index of the led that is closest to the sound source
const int LED_ANGLES[] = {0, 45, 90, 135, 180, 225, 270, 315}; // set angles that led's are positioned at
int ledBrightness[NUM_LEDS]; // brightness for led
const int SAMPLE_WINDOW = 50; // Sample window width in mS (50 mS = 20Hz)
const int PRINT_DELAY = 1000; // print only every 1 second
long printMillis = 0;
const String DIRECTIONS[] = {"right", "front-right", "front", "front-left",
                             "left", "back-left", "back", "back-right"};
const double lowerThreshold = 150;
const String noSoundMsg = "no sound";
const int AVERAGE_WINDOW = 5;
int avgVals[NUM_MICS];
int sums[NUM_MICS];
double avgs[NUM_MICS];
int tempMicVal, tempPeakToPeak = 0;

// get the signals of each mic from 0 to 1023
void getMicVals() {
  long startMillis = millis(); // Start of sample window
  // save the max peak distance during a 50ms window as each mic value
  while (millis() - startMillis < SAMPLE_WINDOW) {
    for (int i = 0; i < NUM_MICS; i++) {
      tempMicVal = analogRead(i); // read the analog in value from mic
      if (tempMicVal < 1024) { // toss out spurious readings
        if (tempMicVal > signalMaxs[i]) {
          signalMaxs[i] = tempMicVal; // save just the max levels
        } 
        if (tempMicVal < signalMins[i]) {
          signalMins[i] = tempMicVal; // save just the min levels
        }
      }
      tempPeakToPeak = signalMaxs[i] - signalMins[i]; // max - min = peak-peak amplitude
      micVals[i] = tempPeakToPeak;
    }
  }
}
// turn all led's off
void turnLedsOff() {
  for (int i = 0; i < NUM_LEDS; i++) {
    ledBrightness[i] = 0;
    analogWrite(LED_PINS[i], ledBrightness[i]);
  }
}
// turn on the closest led
void adjustLeds() {
  turnLedsOff();
  for (int i = 0; i < NUM_LEDS; i++) {
    // if all mics are close to 0, turn all lights off
    if (avgs[0] + avgs[1] + avgs[2] > 18) {
      ledBrightness[indexOfAngle] = 255;
      analogWrite(LED_PINS[indexOfAngle], ledBrightness[indexOfAngle]);
    }
  }
}

// calculate x and y components of the mic vectors (mic1: 30, mic2: 150, mic3: 270) degrees
void findVectorComponents() {
  for (int i = 0; i < NUM_MICS; i++) {
    xComponents[i] = avgs[i] * cos(radians(90) * i);
    yComponents[i] = avgs[i] * sin(radians(90) * i);
  }
}

// find the angle of the sound source in degrees
void findSoundSourceAngle() {
  // combine components of component vectors to find components of resultant vector
  for (int i = 0; i < NUM_MICS; i++) {
    rX += xComponents[i];
    rY += yComponents[i];
  }
  theta = atan2(rY, rX) * 180.0 / PI;
}

void findSmallestAngleDifference() {
  for (int i = 0; i < NUM_LEDS; i++) {
    angleDiff[i] = abs((theta - LED_ANGLES[i]));
    if (angleDiff[i] < smallestAngle) {
      smallestAngle = angleDiff[i];
      indexOfAngle = i;
    }
  }
  smallestAngle = 361; // reset smallest angle for the next loop
}

void findAngleWithTwoGreatestSides(){
  int greatestIndex = 4;
  int secondGreatestIndex = 5;
  double greatestValue = 0;
  double secondGreatestValue = 0;
  for(int i = 0; i < NUM_MICS; i++){ // find mic with greatest value
    if(avgs[i] > greatestValue){
      greatestValue = avgs[i];
      greatestIndex = i;
    }
  }
  for(int i = 0; i < NUM_MICS; i++){ // find mic with second greatest value
    if(avgs[i] > secondGreatestValue && avgs[i] < greatestValue){
      secondGreatestValue = avgs[i];
      secondGreatestIndex = i;
    }
  }
  double x = (avgs[greatestIndex] * cos(radians(90) * greatestIndex)) + (avgs[secondGreatestIndex] * cos(radians(90) * secondGreatestIndex));
  double y = (avgs[greatestIndex] * sin(radians(90) * greatestIndex)) + (avgs[secondGreatestIndex] * sin(radians(90) * secondGreatestIndex));
  theta2 = atan2(y,x) * 180 / PI;
}

void resetValues() {
  for (int i = 0; i < NUM_MICS; i++) {
    micVals[i] = 0;
    signalMaxs[i] = 0;
    signalMins[i] = 1024;
    rX = 0;
    rY = 0;
    sums[i] = 0;
    tempPeakToPeak = 0;
    tempMicVal = 0;
    avgVals[i] = 0;
    avgs[i] = 0;
  }
}

void normalizeAngle() {
  while (theta < 0) {
    theta += 360;
  }
}

void adjustGain(){
  avgs[2] -= 20;
  if(avgs[2] < 0){
    avgs[2] = 0;
  }
}

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  btm.begin(9600);
  printMillis = millis();
}

void loop() {

  resetValues();

  // add multiple values for a more stable output
  for (int m = 0; m < AVERAGE_WINDOW; m++) {
    getMicVals();
    for (int i = 0; i < NUM_MICS; i++) {
      sums[i] += micVals[i];
    }
  }
  // find average values of each mic
  for (int i = 0; i < NUM_MICS; i++) {
    avgs[i] = sums[i] / AVERAGE_WINDOW;
  }

  //adjustGain();

  findVectorComponents();
  findSoundSourceAngle();
  normalizeAngle();
  findSmallestAngleDifference();

  printResults();
  String msg;
  if(avgs[0] < lowerThreshold && avgs[1] < lowerThreshold && avgs[2] < lowerThreshold && avgs[3] < lowerThreshold){
    msg = NO_SOUND_ID + noSoundMsg;
  } else {
    msg = DIRECTION_ID + DIRECTIONS[indexOfAngle];
  }
  btm.println(msg);

  delay(100);
}

void printResults() {
  if (millis() - printMillis >= PRINT_DELAY) {
//    Print("avg0 ", avgs[0]);
//    Print("avg1 ", avgs[1]);
//    Print("avg2 ", avgs[2]);
//    Print("avg3 ", avgs[3]);
//    Serial.println("---------------");
    String tab = "\t";
    Serial.print(avgs[0] + tab);
    Serial.print(avgs[1] + tab);
    Serial.print(avgs[2] + tab);
    Serial.print(avgs[3]);
    Serial.println();
    printMillis = millis();
  }
}

//print the results to the Serial Monitor:
void Print(String start, double msg) {
  String s = start + msg;
  Serial.println(s);
}
