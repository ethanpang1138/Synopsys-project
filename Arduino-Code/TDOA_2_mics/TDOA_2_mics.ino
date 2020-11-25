#include <SoftwareSerial.h>

const int MIC_PINS[] = {0, 1}; // analog pins that mics are connected to
const int NUM_MICS = 2;
SoftwareSerial btm(7,8); // rx tx

String DIRECTION_ID = "00";
String m1ID = "01";
String m2ID = "02";
String m3ID = "03";
String closeID = "**";

char incomingByte; // incoming data
const int NUM_LEDS = 8; // number of leds being used
double theta = 0.0; // angle of the sound source
const double v = 343 / 1000000;
const double d = 14;
boolean timerStarted = false;

double micVals[NUM_MICS];
double prevMicVals[NUM_MICS];

double angleDiff[NUM_LEDS]; // how far the angle of each led is from the sound source
double smallestAngle = 361.0; // holds the current smallest angle difference
int indexOfAngle = 0; // holds the index of the led that is closest to the sound source
const int LED_ANGLES[] = {0, 45, 90, 135, 180, 225, 270, 315}; // set angles that led's are positioned at
const int SAMPLE_WINDOW = 50; // Sample window width in mS (50 mS = 20Hz)
const int PRINT_DELAY = 1000; // print only every 1 second
long printMillis = 0;
const String DIRECTIONS[] = {"right", "front-right", "front", "front-left",
                             "left", "back-left", "back", "back-right"};
const double lowerThreshold = 200;
long startTime = 0;
long elapsedTime = 0;
long microsTime = 0;
int signalMaxs[NUM_MICS]; // temporary max signals from mics
int signalMins[] = {1024, 1024, 1024, 1024}; // temporary min signals from mics
int tempMicVal, tempPeakToPeak = 0;


// get the signals of each mic from 0 to 1023
void getTime() {
  for(int i = 0; i < NUM_MICS; i++){
    prevMicVals[i] = micVals[i];
  }
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
  
  if(!timerStarted && prevMicVals[0] < lowerThreshold && micVals[0] > lowerThreshold){
    startTimer();
  }
  if(timerStarted && prevMicVals[1] < lowerThreshold && micVals[1] > lowerThreshold){
      stopTimer();
  }
}

void startTimer(){
  prevMicVals[0] = micVals[0];
  startTime = micros();
  timerStarted = true;
  Serial.println("timer started");
}
void stopTimer(){
  prevMicVals[1] = micVals[1];
  microsTime = micros();
  elapsedTime = microsTime - startTime;
  timerStarted = false;
  Serial.println("timer stopped");
}

void findAngle(){
  double dD = elapsedTime * v;
  double xb = 7;
  double x = sqrt(-(dD*dD * (dD*dD - 4 * xb*xb))/(4 * (4*xb*xb - dD*dD)));
  double y = sqrt(dD*dD/4 - xb*xb + x*x * (4*xb*xb/(dD*dD) - 1));
  theta = atan2(y,x);
}

void normalizeAngle() {
  while (theta < 0) {
    theta += 360;
  }
}

void reset(){
  for(int i = 0; i < NUM_MICS; i++){
    micVals[i] = 0;
    signalMaxs[i] = 0;
    signalMins[i] = 1024;
    tempPeakToPeak = 0;
    tempMicVal = 0;
  }
}

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  btm.begin(9600);
  printMillis = millis();
}

void loop() {
  reset();
  getTime();
  findAngle();
  printResults();
}

void printResults() {
  if (millis() - printMillis >= PRINT_DELAY) {
    Print("theta: ", theta);
    for(int i = 0; i < NUM_MICS; i++){
      Print("mic: ", micVals[i]);
    }
    Print("start time: ", startTime);
    Print("micros : ", microsTime);
    Print("delta time: ", elapsedTime);
    
    Serial.println(timerStarted);
    Serial.println("------------------------");
    printMillis = millis();
  }
}

//print the results to the Serial Monitor:
void Print(String start, double msg) {
  String s = start + msg;
  Serial.println(s);
}
