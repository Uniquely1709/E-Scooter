#include <Arduino.h>
#include <LiquidCrystal.h>
#include <IoAbstractionWire.h>
#include <Adafruit_BusIO_Register.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <Servo.h>

#pragma region VARS & Definitions
/*
  initialize all MCPS
*/
Adafruit_MCP23X17 mcp1; 
Adafruit_MCP23X17 mcp2;  // Used for REMOTE AND DISPLAY

Servo lenker; 
int lenkerPos = 0;    

#define DebugPin 5
#define DebugLed 4
/*
  Define all pins used for Remote and Motor Control
 */
#define CH1 9 //Channels for remote
#define CH2 6
#define CH3 5
#define CH4 3
#define pwmPin 10 //PWM for hacked motor driver

#define ReceiveLED 9

/*
    Define Racing Panel Input
*/
#define EngineStart 7
#define SwitchMaster  3
#define Switch1 6
#define Switch2 5
#define Switch3 4

#define BlinkerLinks 7
#define BlinkerRechts 6
 
/*
  Storages 
*/
int ch1Value;
int ch3Value;
int ch4Value, ch4LastValue; 
int multi; 
int counter = 0; 
int time = 0; 
bool ch2Value;
bool debug = false; 

/*
  defining all needed for display
*/
const int rs = 2, en = 4, d4 = 7, d5 = 11, d6 = 12, d7 = 13;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int pBari = 0;         // progress bar
byte pBar[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

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

void controlLenker(int val){
  if (val == 0){
    lenker.write(90);

  }else{
    int mapped = map(val, 10, 255, 90,180);
    Serial.println(mapped);
    lenker.write(mapped);
  }

  // if (val == 0){
  //   lenker.write(135);
  // }
  // lenker.write(val);
}

void displaySpeed(int speed, int multi){
  lcd.clear();
  lcd.setCursor(0,0);

  pBari=map(multi, 0, 255, 0, 17);
  //prints the progress bar
  for (int i=0; i < pBari; i++)
  {
    lcd.setCursor(i, 0);   
    lcd.write(byte(0));  
  };

  lcd.setCursor(0,1);

  pBari=map(speed, 0, 255, 0, 17);
  //prints the progress bar
  for (int i=0; i < pBari; i++)
  {
    lcd.setCursor(i, 1);   
    lcd.write(byte(0));  
  };
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
 
#pragma endregion 

#pragma region Setup
void setup(){
  // Set up serial monitor
  Serial.begin(115200);
  lenker.attach(8);

  // Set all pins as inputs for Remote
  pinMode(CH1, INPUT);
  pinMode(CH2, INPUT);
  pinMode(CH3, INPUT);
  pinMode(CH4, INPUT);

  // Set pin for PWM Motor control
  pinMode(pwmPin, OUTPUT);

  lcd.begin(16, 2);
  lcd.print("First line");
  lcd.setCursor(0,1);
  lcd.print("Second line");
  //Create the progress bar character
  lcd.createChar(0, pBar);

  mcp1.begin_I2C(0x20);      // Start MCP 1
  mcp2.begin_I2C(0x24);      // Start MCP 2
  
  mcp1.pinMode(ReceiveLED, OUTPUT); // Define GPB0 on MCP1 as Output
  mcp1.digitalWrite(ReceiveLED, LOW);

  mcp1.pinMode(EngineStart, INPUT);
  mcp1.pinMode(SwitchMaster, INPUT);
  mcp1.pinMode(Switch1, INPUT);
  mcp1.pinMode(Switch2, INPUT);
  mcp1.pinMode(Switch3, INPUT);

  mcp2.pinMode(BlinkerLinks, OUTPUT);
  mcp2.digitalWrite(BlinkerLinks, LOW);
  mcp2.pinMode(BlinkerRechts, OUTPUT);
  mcp2.digitalWrite(BlinkerRechts, LOW);

  mcp2.pinMode(DebugPin, INPUT);
  mcp2.pinMode(DebugLed, OUTPUT);
  mcp2.digitalWrite(DebugLed, LOW);
}

#pragma endregion 
 
#pragma region Loop

void loop() {

  /*
    Control DEBUG STATE & Visual representation through a LED
  */
  if (mcp2.digitalRead(DebugPin) == HIGH){
    debug = !debug; 
  }
  mcp2.digitalWrite(DebugLed, debug);
  
  // Get values for each channel
  ch1Value = readChannel(CH1, -255, 255, 0);
  ch2Value = readSwitch(CH2, false);
  ch3Value = readChannel(CH3, -255, 255, 0);
  ch4Value = readChannel(CH4, -255, 255, 0);
  multi = multiMap(ch3Value, in, s, 12);
  int sum = ch1Value + ch2Value + ch3Value + ch4Value; 
  
  // Print to Serial Monitor
  if (debug){
    Serial.print("Ch1: ");
    Serial.print(ch1Value);
    Serial.print(" | Ch2: ");
    Serial.print(ch2Value);
    Serial.print(" | Ch3: ");
    Serial.print(ch3Value);
    Serial.print(" | Mapped: ");
    Serial.print(multi);
    Serial.print(" | Ch4: ");
    Serial.println(ch4Value); 
  }


  // Write calculated value to motor driver 
  if (ch3Value >= 0) {
    writeSpeedToMotor(ch3Value);
    displaySpeed(ch3Value, multi);
  }

  if(ch1Value > 10){
    mcp2.digitalWrite(BlinkerLinks, HIGH);
  }else{
    mcp2.digitalWrite(BlinkerLinks, LOW);
  }
  if(ch1Value < -10){
    mcp2.digitalWrite(BlinkerRechts, HIGH);
  }else{
    mcp2.digitalWrite(BlinkerRechts, LOW);
  }


  if(((ch4LastValue +10) <= ch4Value)or (ch4Value > 25)){
    controlLenker(ch4Value);
  }else if (ch4Value == 0){
    controlLenker(ch4Value);
  }
  if(((ch4LastValue -10) <= ch4Value)or (ch4Value < -25)){
    controlLenker(ch4Value);
  }else if (ch4Value == 0){
    controlLenker(ch4Value);
  }


  if (sum != 0 ){
    mcp1.digitalWrite(ReceiveLED, HIGH);
  }else {
    mcp1.digitalWrite(ReceiveLED, LOW);
  }

  if (mcp1.digitalRead(EngineStart)== HIGH){ 
    Serial.println("ENGINE START");
  }
  // if (mcp1.digitalRead(28) == LOW){
  //   Serial.println("pressed");
  // }else{
  //   Serial.println(" not pressed");

  // }

  // if (ch1Value > 50 ){
  //   Serial.println("HIGH");
  //   mcp1.digitalWrite(28, HIGH);
  // }else{
  //   mcp1.digitalWrite(28, LOW);
  // }
  // time = time + 500; 
  ch4LastValue = ch4Value; 
  delay(100);
}
#pragma endregion