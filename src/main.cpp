#include <Arduino.h>
#include <LiquidCrystal.h>
#include <IoAbstractionWire.h>
#include <Adafruit_BusIO_Register.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <Servo.h>

#pragma region VARS & Definitions

/*
  Define all pins used for Remote and Motor Control
 */
#define CH1 3 //Channels for remote
#define CH2 5
#define CH3 6
#define CH4 9
#define pwmPin 10 //PWM for hacked motor driver

/*
  Storages 
*/
int ch1Steering;
int ch2Gas;
int ch4Indicator; 
int GoalSpeed, ActualSpeed, LastActualSpeed; 
bool ch3Button;
bool debug = false; 

/*
  Defining mapping for S Curve to normalize Speed Control
*/
int s[] = {0,23, 46, 69, 92, 115, 138, 161, 184, 207, 230, 255};
int in[] = {0, 48, 69, 88, 97, 118, 135, 154, 174, 202, 222, 255}; 

#pragma endregion

#pragma region functions

// Read the number of a specified channel and convert to the range provided.
// If the channel is off, return the default value
int readChannel(int channelInput, int minLimit, int maxLimit, int defaultValue){
  int ch = pulseIn(channelInput, HIGH, 30000);
  if (ch < 100) return defaultValue;
  int mapped = map(ch, 875, 2140, minLimit, maxLimit);
  if ((mapped < 10) && (mapped > -10)){
    return 0;
  }else{
    return mapped;
  } 
}

// Read the switch channel and return a boolean value
bool readSwitch(byte channelInput, bool defaultValue){
  int intDefaultValue = (defaultValue)? 100: 0;
  int ch = readChannel(channelInput, 0, 100, intDefaultValue);
  return (ch > 50);
}

void writeSpeedToMotor(int speed){
  analogWrite(pwmPin, speed);
}

// note: the _in array should have increasing values
int multiMap(int val, int* _in, int* _out, uint8_t size)
{
  // take care the value is within range
  // val = constrain(val, _in[0], _in[size-1]);
  if (val <= _in[0]) return _out[0];
  if (val >= _in[size-1]) return _out[size-1];

  // search right interval
  uint8_t pos = 1;  // _in[0] allready tested
  while(val > _in[pos]) pos++;

  // this will handle all exact "points" in the _in array
  if (val == _in[pos]) return _out[pos];

  // interpolate in the right segment for the rest
  return (val - _in[pos-1]) * (_out[pos] - _out[pos-1]) / (_in[pos] - _in[pos-1]) + _out[pos-1];
}

int calculateSpeed(int goal, int actual){
  if (actual < goal && (actual + 15) < goal){
    return actual + 15;
  }else {
    return goal; 
  }
}

#pragma endregion

#pragma region Setup

void setup(){
  // Set up serial monitor
  Serial.begin(115200);

  // Set all pins as inputs for Remote
  pinMode(CH1, INPUT);
  pinMode(CH2, INPUT);
  pinMode(CH3, INPUT);
  pinMode(CH4, INPUT);

  // Set pin for PWM Motor control
  pinMode(pwmPin, OUTPUT);

}

#pragma endregion

#pragma region Loop

void loop() {

  // Get values for each channel
  ch1Steering = readChannel(CH1, -255, 255, 0);
  ch2Gas = readChannel(CH2, -255, 255, 0);
  ch3Button = readSwitch(CH3, false);
  ch4Indicator = readChannel(CH4, -255, 255, 0);
  GoalSpeed = multiMap(ch2Gas, in, s, 12);
  ActualSpeed = calculateSpeed(GoalSpeed, LastActualSpeed);
  int sum = ch1Steering + ch2Gas + ch3Button + ch4Indicator; 

  Serial.print("Ch1Steering: ");
  Serial.print(ch1Steering);
  Serial.print(" | Ch2Gas: ");
  Serial.print(ch2Gas);
  Serial.print(" | GoalSpeed: ");
  Serial.print(GoalSpeed);
  Serial.print(" | AcutalSpeed: ");
  Serial.print(ActualSpeed);
  Serial.print(" | Ch3Button: ");
  Serial.print(ch3Button);
  Serial.print(" | Ch4Indicator: ");
  Serial.println(ch4Indicator);

  // Write calculated value to motor driver 
  if (ActualSpeed >= 0) {
    writeSpeedToMotor(ActualSpeed);
    //displaySpeed(ch3Value, multi);
  }

  LastActualSpeed = ActualSpeed; 
  delay(500);
}

#pragma endregion
