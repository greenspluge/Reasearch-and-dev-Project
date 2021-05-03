#include <Wire.h> // includes the arduino wire library
#include "rgb_lcd.h" // includes seed studio LCD library
#include<Servo.h>


rgb_lcd lcd; // initialises LCD library

//Declaring All output and input pins
const int led_Pin = 2;
const int button_Pin = 3; 
const int trig_Pin = 5;
const int echo_Pin = 4;
const int buzzer_Pin = 6;
const int button_Pin_Alarm = 7;
const int alarm_led_Pin = 8;
const int motionReading = 10;
const int servo_Pin = 11;
Servo ServoMotor;
//Declaring all Analog pins
const int analog_Light_Pin = A0;
const int analog_tmp_Pin = A1;
//Declaring thresholds used for sensor system
const int LowLightThreshold = 600;
const int LowTempThreshold = 12;
const int HighTempThreshold = 25;
//Declaring initial system states
boolean alarm_OnOrOff = false;
boolean lightOnByButton = false;
boolean lightOnByNight = false;


void setup() {
  // put your setup code here, to run once:
  // initialise all the pin types
  Serial.begin(9600);
  pinMode(led_Pin, OUTPUT);
  pinMode(trig_Pin, OUTPUT);
  pinMode(buzzer_Pin, OUTPUT);
  pinMode(echo_Pin, INPUT);
  pinMode(button_Pin, INPUT);
  pinMode(button_Pin_Alarm, INPUT);
  pinMode(alarm_led_Pin, OUTPUT);
  ServoMotor.attach(servo_Pin);
  
  lcd.begin(16, 2); // inistialises the LCD with 16 columns and 2 rows
  lcd.setCursor(0, 0); // moves cursor to starting top left posistion.
}

// measuring Distance using ultrasonic sensor
long measureDistance(){
  //settle ultrasonic sensor
  digitalWrite(trig_Pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trig_Pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_Pin, LOW);
  pinMode(echo_Pin, INPUT);
  //detect time for a pulse to come back
  long duration = pulseIn(echo_Pin, HIGH);
  long cm = (duration/2) / 29.1;
  //convert time to a distance
  return cm;
}

int measureLight(){
  return analogRead(analog_Light_Pin);
  //measures the light by taking reading from light sensor
}


void rotateTo90(){
  ServoMotor.write(90);
  //opens the vent
}

void rotateTo0(){
  ServoMotor.write(0);
  //closes the vent
}

void turnBuzzerOn(){
  digitalWrite(buzzer_Pin,HIGH);  
  //turns buzzer on  
}

void turnBuzzerOff(){
  digitalWrite(buzzer_Pin,LOW);  
  //turns buzzer off
}
void turnOnLEDS(){
  digitalWrite(led_Pin,HIGH);
  //turns leds on
}

void turnOffLEDS(){
  digitalWrite(led_Pin,LOW);
  //turns leds off
}

float readTemperatureOffSensor(){
  float tmpReading = analogRead(analog_tmp_Pin);
  //Takes temperature reading
  float voltage = tmpReading * 5.0;
  voltage /= 1024.0;
  float TmpInC = (voltage - 0.5) * 100;
  // converts analog input into degrees C for user output
  return TmpInC;
}

//alarms on
void alarmLoop(){
  //variable for the button state
  int buttonState = 0;
  //vairable for the loop
  boolean loopUntil = true;
  //reads button state
  buttonState = digitalRead(button_Pin);
  while(loopUntil){
    buttonState = digitalRead(button_Pin);
    if(buttonState == HIGH){
      loopUntil = false;
      turnBuzzerOff();
      delay(2000);
    }else{
      //Turn the buzzer on and blink LED
      turnOnLEDS();    
      turnBuzzerOn();
      delay(1000);  
      turnOffLEDS();
      delay(1000);
    }
  }
}

void NightTimeAlarm(){
  int lightReading = measureLight();
  Serial.println(lightReading);
  //If Light reading is low then turn on motion sensor
  if(lightReading < LowLightThreshold){
    alarm_OnOrOff = true;
    digitalWrite(alarm_led_Pin,HIGH);
    //check for person in green house
    int distance = measureDistance();
    Serial.println(distance);
    if(measureDistance() < motionReading){
      if(alarm_OnOrOff){
        alarmLoop();         
      }
    }
  }
}

void nightLight(){
  //measure light value
  int lightReading = measureLight();
  //char txt[] = lightReading;
  lcd.setCursor(0, 1);
  lcd.print(lightReading);
  //Serial.println(lightReading);
  //if theres low light turn LEDS on
  if(lightReading < LowLightThreshold){
    turnOnLEDS(); 
    lightOnByNight = true;   
  }else if(!lightOnByButton){//Turn leds off if theres light
    turnOffLEDS();    
  }
}

void checkTemperature(){
  float lightReading = measureLight();
  //if theres high light check the temperature
  if(lightReading >= LowLightThreshold){
    //read temperature
    float tempReading = readTemperatureOffSensor();
    //char txt[] = tempReading;
    lcd.setCursor(8, 1);
    lcd.print(tempReading);
    //Serial.println(tempReading);
    //if theres a low temperature turn buzzer on and close vent
    if (tempReading <= LowTempThreshold){
      turnBuzzerOn();
      rotateTo0();            
    }else if(tempReading >= HighTempThreshold){//if theres a high temperature open the vent
      rotateTo90();      
    }else{//Turn buzzer off and close the vent
      turnBuzzerOff();
      rotateTo0();
    }      
  }
}

void checkLightButton(){
  //read button reading
  int buttonState = digitalRead(button_Pin);
  //if buttons pressed change light state
  if(buttonState == HIGH){
    //if the lights on then turn light of if its not dark
    if(lightOnByButton){
      lightOnByButton = false;  
      if(!lightOnByNight){
        turnOffLEDS();               
      }
    //if the lights are off turn LED's on           
    }else{
      turnOnLEDS(); 
      lightOnByButton = true;       
    }
  }
}

void check_Alarm_Button(){
  //read button reading
  int buttonState = digitalRead(button_Pin_Alarm);
  //if buttons pressed change alarm state
  if(buttonState == HIGH){
    //change alarm state to true/false if false/true
    if(alarm_OnOrOff = true){
      //switch state
      alarm_OnOrOff = false;
      digitalWrite(alarm_led_Pin,LOW);  
    }else{
      alarm_OnOrOff = true;
      digitalWrite(alarm_led_Pin,HIGH);
    }
    delay(1000);
  } 
}
void loop() {
  //checks for intrusion if dark
  NightTimeAlarm();
  delay(1000);
  //Turns the lights on if its dark
  nightLight();
  delay(1000);
  //If its dark and temperatures low turn buzzer on or if temperature open the flap
  checkTemperature();
  delay(1000);
  //if the buttons pressed turn the light on/ off (don't turn light off if its dark)
  checkLightButton();
  //if alarm buttons pressed change alarm state
  check_Alarm_Button();

}
