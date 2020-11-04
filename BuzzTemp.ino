#include "max6675.h"  //INCLUDE THE LIBRARY
#include <LiquidCrystal.h>

//LCD pin to Arduino
const int pin_RS = 8;
const int pin_EN = 9;
const int pin_d4 = 4;
const int pin_d5 = 5;
const int pin_d6 = 6;
const int pin_d7 = 7;
const int pin_BL = 10;
LiquidCrystal lcd( pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);


int thermoDO = 13;
int thermoCS = 12;
int thermoCLK = 11;


// define some values used by the panel and buttons
int lcd_key     = 6;
int adc_key_in  = 0;

#define btnRIGHT  1
#define btnUP     2
#define btnDOWN   3
#define btnLEFT   4
#define btnSELECT 5
#define btnNONE   6

// read the buttons
int read_LCD_buttons()
{
  adc_key_in = analogRead(0);      // read the value from the sensor
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1500) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 195)  return btnUP;
  if (adc_key_in < 380)  return btnDOWN;
  if (adc_key_in < 500)  return btnLEFT;
  if (adc_key_in < 700)  return btnSELECT;
  return btnNONE;  // when all others fail, return this...
}

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

char buffer[16];

int buzzer = A5;
int freq = 1000;

double alarmTemp = 27.00;
char debouceAlarmTemp = 0;
int debouceAlarmTempThr = 4;
double graceTemp = 1;

bool isTempInAlarm, isAlarmCleared;
double currentTemp;



void setup() {
  Serial.begin(9600);
  Serial.println("MAX6675 test");
  lcd.begin(16, 2);

  pinMode(buzzer, OUTPUT); // Set buzzer - pin 9 as an output
    noTone(buzzer);


  // wait for MAX chip to stabilize
  delay(500);

  isAlarmCleared = false;
  isTempInAlarm = false;
}

void loop() {


  currentTemp = thermocouple.readCelsius();
   lcd_key = read_LCD_buttons();  // read the buttons



  if (abs(currentTemp - alarmTemp) > graceTemp) {
    debouceAlarmTemp ++;
  }
  else {
    debouceAlarmTemp=0;
    isAlarmCleared = false;
  }

  if(debouceAlarmTemp > debouceAlarmTempThr){
    isTempInAlarm = true;
  }
  else {
    isTempInAlarm = false;
  }



  switch (lcd_key)               // depending on which button was pushed, we perform an action
  {
    case btnRIGHT:
      {
        isAlarmCleared = true;
        break;
      }
    case btnLEFT:
      {
        isAlarmCleared = true;
        break;
      }
    case btnUP:
      {
        isAlarmCleared = true;
        break;
      }
    case btnDOWN:
      {
        isAlarmCleared = true;
        break;
      }
    case btnSELECT:
      {
        isAlarmCleared = true;
        break;
      }
    case btnNONE:
      {
        break;
      }
  }

  lcd.setCursor(0, 1);

  lcd.print("C= ");
  lcd.print(thermocouple.readCelsius(), 2);


  if (!isAlarmCleared && isTempInAlarm) {
    tone(buzzer, freq);
  }
  else {
    noTone(buzzer);

  }
  delay(500);
}
