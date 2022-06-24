#include <Arduino.h>
#include <LiquidCrystal.h>

// Define Input Connections
#define CH1 3
#define CH2 5
#define CH3 6
#define CH4 9
#define pwmPin 2
 
// Integers to represent values from sticks and pots
int ch1Value;
int ch3Value;
int ch4Value; 
int multi; 
int counter = 0; 
int time = 0; 
 
// Boolean to represent switch value
bool ch2Value;

const int rs = 7, en = 8, d4 = 10, d5 = 11, d6 = 12, d7 = 13;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
//progress bar character for brightness
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

//S curve
int s[] = {0,23, 46, 69, 92, 115, 138, 161, 184, 207, 230, 255};
int in[] = {0, 48, 69, 88, 97, 118, 135, 154, 174, 202, 222, 255}; 
 
// Read the number of a specified channel and convert to the range provided.
// If the channel is off, return the default value
int readChannel(int channelInput, int minLimit, int maxLimit, int defaultValue){
  int ch = pulseIn(channelInput, HIGH, 30000);
  if (ch < 100) return defaultValue;
  int mapped = map(ch, 875, 2140, minLimit, maxLimit);
  if ((mapped < 5) && (mapped > -5)){
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

  lcd.begin(16, 2);
  lcd.print("First line");
  lcd.setCursor(0,1);
  lcd.print("Second line");
  //Create the progress bar character
  lcd.createChar(0, pBar);
}
 
void loop() {
  
  // Get values for each channel
  ch1Value = readChannel(CH1, -255, 255, 0);
  ch2Value = readSwitch(CH2, false);
  ch3Value = readChannel(CH3, -255, 255, 0);
  ch4Value = readChannel(CH4, -255, 255, 0);
  multi = multiMap(ch3Value, in, s, 12);
  
  // Print to Serial Monitor
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

  
  // if (ch3Value > 5 ){
  //     if (counter <= 10){
  //       Serial.print(time);
  //       Serial.print(" | ");
  //       Serial.println(ch3Value);
  //     }
  //     counter ++;
  // }


  


  // Write calculated value to motor driver 
  if (ch3Value >= 0) {
    writeSpeedToMotor(ch3Value);
    displaySpeed(ch3Value, multi);
  }
  time = time + 500; 
  delay(500);
}