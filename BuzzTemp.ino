#define TEMP_MAX6675_TIPE_K 0x01
#define TEMP_DS18B20 0x02

/**
 *  Your definitions here
 */

/**
 *  Serial baud rate
 */
#define SERIAL_BAUD_RATE 9600

/**
 *  Default target temperature
 */
#define DEFAULT_TARGET_TEMP 27.00

/**
 * Default target temperature
 */
#define DEFAULT_GRACE_RANGE_TEMP 1.00

/**
 *  Temperature sensor to use
 *  + TEMP_DS18B20 -> one wire DS18B20 sensor
 *  + TEMP_MAX6675_TIPE_K -> SPI MAX6675 with type K temp. probe
 */
#define TEMP_SENSOR TEMP_MAX6675_TIPE_K
#if TEMP_SENSOR==TEMP_DS18B20
/**
 *  Data pin where DS18B20 is connected
 */
#define TEMP_DS18B20_DATA_PIN 11
#endif;
#if TEMP_SENSOR==TEMP_MAX6675_TIPE_K
/**
 * SPI pins where MAX6675 is connected (DATA OUT + CHIP SELCT + CLOCK)
 */
#define TEMP_MAX6675_DATA_OUT_PIN 13
#define TEMP_MAX6675_CHIP_SELECT_PIN 12
#define TEMP_MAX6675_CLOCK_PIN 11
#endif;

/**
 * Pin where buzzer is connected
 */
#define BUZZER_PIN A5

/**
 * The frequecy of the tone of the buzzer
 */
#define BUZZER_TONE_FREQUENCY 1200

/**
 *  END of your definitions here
 */

#if TEMP_SENSOR==TEMP_MAX6675_TIPE_K
#include "max6675.h"
MAX6675 thermocouple(TEMP_MAX6675_CLOCK_PIN, TEMP_MAX6675_CHIP_SELECT_PIN, TEMP_MAX6675_DATA_OUT_PIN);
#endif

#if TEMP_SENSOR==TEMP_DS18B20
#include <DallasTemperature.h>
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(TEMP_DS18B20_DATA_PIN);

DallasTemperature sensors(&oneWire);


#endif



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

char buffer[16];


double alarmTemp = DEFAULT_TARGET_TEMP;
char debouceAlarmTemp = 0;
int debouceAlarmTempThr = 4;
double graceTemp = DEFAULT_GRACE_RANGE_TEMP;

bool isTempInAlarm, isAlarmCleared, isRun;
double currentTemp, currentCurrentReading;
short int currentReadingCounter;


#define LOOP_DELAY 300

#define SAMPLING_COUNT 1

#define CURSOR_POSITION_TARGET_INT 0
#define CURSOR_POSITION_TARGET_DEC 1
#define CURSOR_POSITION_HYSTERESIS_INT 2
#define CURSOR_POSITION_HYSTERESIS_DEC 3
#define CURSOR_POSITION_RUN 4

int cursorAllowedPositionIndex = 0;

short int allowedPositions = 5;
int allowedCursorPositions[5][2] = {
  {3, 0}, //target unit
  {5, 0}, //target decimal
  {12, 0}, //histeresis unit
  {14, 0}, //histeresis decimal
  {15, 1}, //run
};


void updateLcdData() {
  lcd.setCursor(0, 0);

  lcd.print("T ");
  lcd.print(alarmTemp, 2);

  lcd.setCursor(10, 0);
  lcd.print("I ");
  lcd.print(graceTemp, 2);
  lcd.setCursor(0, 1);
  lcd.print("C");
  if (isTempInAlarm) {
    lcd.print("!");
  }
  else {
    lcd.print(" ");
  }
  lcd.print(currentTemp, 2);
  lcd.setCursor(10, 1);
  lcd.print("S  ");
  if (isRun) {
    lcd.print("RUN");
  }
  else {
    lcd.print("OFF");
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("BuzzTemp Starting");
  lcd.begin(16, 2);

  pinMode(BUZZER_PIN, OUTPUT); // Set buzzer - pin 9 as an output
  noTone(BUZZER_PIN);


  // wait for MAX chip to stabilize

  isAlarmCleared = false;
  isTempInAlarm = false;

  lcd.setCursor(0, 0);
  lcd.blink();




  lcd.setCursor(allowedCursorPositions[cursorAllowedPositionIndex][0], allowedCursorPositions[cursorAllowedPositionIndex][1]);

  delay(500);
#if TEMP_SENSOR==TEMP_DS18B20
  sensors.begin();
  sensors.requestTemperatures();
  currentTemp = sensors.getTempCByIndex(0);
#endif
#if TEMP_SENSOR==TEMP_MAX6675_TIPE_K
  currentTemp = thermocouple.readCelsius();
#endif

  delay(500);
}

void loop() {

#if TEMP_SENSOR==TEMP_MAX6675_TIPE_K
  currentTemp += thermocouple.readCelsius();
#endif
#if TEMP_SENSOR==TEMP_DS18B20
  sensors.requestTemperatures();
  currentCurrentReading += sensors.getTempCByIndex(0);
#endif
  currentReadingCounter ++;
  if (currentReadingCounter >= SAMPLING_COUNT) {
    currentTemp = currentCurrentReading / SAMPLING_COUNT;
    currentCurrentReading = 0;
    currentReadingCounter = 0;
  }


  lcd_key = read_LCD_buttons();  // read the buttons



  if (isRun && abs(currentTemp - alarmTemp) > graceTemp) {
    if (!isTempInAlarm) {
      debouceAlarmTemp ++;
    }
  }
  else {
    debouceAlarmTemp = 0;
    isAlarmCleared = false;
  }

  if (debouceAlarmTemp > debouceAlarmTempThr) {
    isTempInAlarm = true;
  }
  else {
    isTempInAlarm = false;
  }



  switch (lcd_key)               // depending on which button was pushed, we perform an action
  {
    case btnRIGHT:
      {
        if (isTempInAlarm && !isAlarmCleared) {
          isAlarmCleared = true;
        }
        else {
          cursorAllowedPositionIndex = (cursorAllowedPositionIndex + 1)  % allowedPositions;
        }
        break;
      }
    case btnLEFT:
      {
        if (isTempInAlarm && !isAlarmCleared) {
          isAlarmCleared = true;
        }
        else {
          cursorAllowedPositionIndex = (cursorAllowedPositionIndex - 1)  % allowedPositions;
        }
        break;
      }
    case btnUP:
      {
        if (isTempInAlarm && !isAlarmCleared) {
          isAlarmCleared = true;
        }
        else {
          switch (cursorAllowedPositionIndex) {
            case CURSOR_POSITION_TARGET_INT:
              alarmTemp += 1;
              break;
            case CURSOR_POSITION_TARGET_DEC:
              alarmTemp += 0.1;
              break;
            case CURSOR_POSITION_HYSTERESIS_INT:
              graceTemp += 1;
              break;
            case CURSOR_POSITION_HYSTERESIS_DEC:
              graceTemp += 0.1;
              break;
            case CURSOR_POSITION_RUN:
              isRun = !isRun;
              break;
          }
        }
        break;
      }
    case btnDOWN:
      {
        if (isTempInAlarm && !isAlarmCleared) {
          isAlarmCleared = true;
        }
        else {
          switch (cursorAllowedPositionIndex) {
            case CURSOR_POSITION_TARGET_INT:
              alarmTemp -= 1;
              break;
            case CURSOR_POSITION_TARGET_DEC:
              alarmTemp -= 0.1;
              break;
            case CURSOR_POSITION_HYSTERESIS_INT:
              graceTemp -= 1;
              break;
            case CURSOR_POSITION_HYSTERESIS_DEC:
              graceTemp -= 0.1;
              break;
            case CURSOR_POSITION_RUN:
              isRun = !isRun;
              break;
          }
        }
        break;
      }
    case btnSELECT:
      {
        if (isTempInAlarm && !isAlarmCleared) {
          isAlarmCleared = true;
        }
        break;
      }
    case btnNONE:
      {
        break;
      }
  }

  updateLcdData();


  if (!isAlarmCleared && isTempInAlarm) {
    tone(BUZZER_PIN, BUZZER_TONE_FREQUENCY);
  }
  else {
    noTone(BUZZER_PIN);

  }
  lcd.setCursor(allowedCursorPositions[cursorAllowedPositionIndex][0], allowedCursorPositions[cursorAllowedPositionIndex][1]);

  delay(LOOP_DELAY);
}
