const int MIC_PINS[] = {0,1,2}; // analog pins that mics are connected to
int micVals[] = {0,0,0}; // values read from mics (0 to 1023)
int signalMaxs[] = {0,0,0}; // temporary max signals from mics
int signalMins[] = {1024,1024,1024};// temporary min signals from mics
double xComponents[] = {0,0,0}; // x components of mic vectors
double yComponents[] = {0,0,0}; // y components of mic vectors
const int LED_PINS[] = {3,5,6,7,8,9,10,11}; //analog output ports for led's (right to left)
const int NUM_LEDS = 8; // number of leds being used
double rX, rY = 0.0; // x and y components for the two vectors and the resultant vector , the angle of the resultant
double theta = 0.0; // angle of the sound source
double angleDiff[] = {0,0,0,0,0,0,0,0}; // how far the angle of each led is from the sound source
double smallestAngle = 361.0; // holds the current smallest angle difference
int indexOfAngle = 0; // holds the index of the led that is closest to the sound source
const int LED_ANGLES[] = {0,45,90,135,180,225,270,315}; // set angles that led's are positioned at
int ledBrightness[] = {0,0,0,0,0,0,0,0}; // brightness for led
const int SAMPLE_WINDOW = 50; // Sample window width in mS (50 mS = 20Hz)
const int PRINT_DELAY = 1000; // print only every 1 second
long printMillis = 0;
const String DIRECTIONS[] = {"right/green","front-right/red", "front/blue", "front-left/red", 
                             "left/green", "back-left/yellow", "back/blue", "back-right/red"};

// get the signals of each mic from 0 to 1023
void getMicVals(){
  long startMillis= millis(); // Start of sample window
  int micVal = 0;
  int peakToPeak = 0;
  // save the max peak distance during a 50ms window as each mic value
  while (millis() - startMillis < SAMPLE_WINDOW){
    for(int i = 0; i < 3; i++){
      micVal = analogRead(i); // read the analog in value from pot
      if (micVal < 1024){ // toss out spurious readings
        if (micVal > signalMaxs[i]){
          signalMaxs[i] = micVal; // save just the max levels
        } else if (micVal < signalMins[i]){
          signalMins[i] = micVal; // save just the min levels
        }
      }
      peakToPeak = signalMaxs[i] - signalMins[i]; // max - min = peak-peak amplitude
      micVals[i] = peakToPeak;
    }
  }
}
// turn all led's off
void turnLedsOff(){
  for(int i = 0; i < NUM_LEDS; i++){
    ledBrightness[i] = 0;
    analogWrite(LED_PINS[i], ledBrightness[i]);
  }
}
// turn on the closest led
void adjustLeds(){
  turnLedsOff();
  for(int i = 0; i < NUM_LEDS; i++){
    // if all mics are close to 0, turn all lights off
    if(micVals[0] + micVals[1] + micVals[2] > 18){
      ledBrightness[indexOfAngle] = 255;
      analogWrite(LED_PINS[indexOfAngle], ledBrightness[indexOfAngle]);
    }
  }
}

// calculate x and y components of the mic vectors (mic1: 30, mic2: 150, mic3: 270) degrees
void findVectorComponents(){
  for(int i = 0; i < 3; i++){
    xComponents[i] = micVals[i] * cos(radians(30) + (radians(120) * i));
    yComponents[i] = micVals[i] * sin(radians(30) + (radians(120) * i));
  }
}

// find the angle of the sound source in degrees
void findSoundSourceAngle(){
  // combine components of component vectors to find components of resultant vector
  for(int i = 0; i < 3; i++){
    rX += xComponents[i];
    rY += yComponents[i];
  }
  theta = atan2(rY, rX) * 180.0 / PI;
}

void findSmallestAngleDifference(){
  for(int i = 0; i < NUM_LEDS; i++){
    angleDiff[i] = abs((theta - LED_ANGLES[i]));
    if(angleDiff[i] < smallestAngle){
      smallestAngle = angleDiff[i];
      indexOfAngle = i;
    }
  }
  smallestAngle = 361; // reset smallest angle for the next loop
}

void resetValues(){
  for(int i = 0; i < 3; i++){
    micVals[i] = 0;
    signalMaxs[i] = 0;
    signalMins[i] = 1024;
    rX = 0;
    rY = 0;
  }
}

void normalizeAngle(){
  while(theta < 0){
    theta += 360;
  }
}

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  printMillis = millis();
}

void loop() {
  
  resetValues();
  getMicVals();
  findVectorComponents();
  findSoundSourceAngle();
  normalizeAngle();
  findSmallestAngleDifference();
  adjustLeds();

  if(millis() - printMillis >= PRINT_DELAY){
    Print("mic1: ", micVals[0]);
    Print("mic2: ", micVals[1]);
    Print("mic3: ", micVals[2]);
    Print("angle: ", theta);
    Serial.println("Should be: " + DIRECTIONS[indexOfAngle]);
    Serial.println("------------");
    printMillis = millis();
  }
  delay(2); // delay for 1 second
}

//print the results to the Serial Monitor:
void Print(String start, double msg){
  String s = start;
  Serial.println(s + msg);
}
  
