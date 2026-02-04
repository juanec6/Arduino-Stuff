// -- LIBRARIES
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Servo.h>

// -- COMPONENTS
#define RED_LED 5
#define GREEN_LED 4
#define SERVO_R 2
#define BACK_BUTTON 6
#define ENTER_BUTTON 3
#define JOY_X A0
#define JOY_Y A1
#define DEADZONE 30
#define SWITCHER_RS 9

// -- CLASSES
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;
Servo Servo1;

// -- MENU OPTIONS
const uint8_t numOptions = 5;
const char* menu[numOptions] = {
  "Radar signal",
  "Radar logs",
  "Manual Radar",
  "Sound signal",
  "Radar and Sound"
};


void setup()
{
  Serial.begin(9600);
  // -- LCD initialization
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(F("Hola mundo feo"));
  delay(2000);
  lcd.clear();

  // -- RTC initialization
  if(rtc.begin() == false)
  {
    lcd.print(F("Error RTC"));
    while(1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // -- Leds initialization
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, HIGH);

  // -- Servo1 initialization
  Servo1.attach(SERVO_R);
  Servo1.write(0);

  // -- Back & Enter buttons
  pinMode(ENTER_BUTTON, INPUT_PULLUP);
  pinMode(BACK_BUTTON, INPUT_PULLUP);

  // -- Servo and Radar ON/OFF switcher. 
  pinMode(SWITCHER_RS, INPUT_PULLUP);
}

// -- FUNCTIONS AND VARIABLES OF COMPONENTS
// -- Servo and radar variables
int16_t servoPos = 0;
int8_t servoDir = 1;
// -- This is done to avoid delays, basically what happens is that
unsigned long timeStamp = 0;
void servoMotionAndRadar(bool ON, int8_t step) // If ON is True, Servo and radar will work, if false, it wont. 
{
  if(ON == true) // Turn on servo. 
  {
    // we don't want the servo to move like crazy, so it makes a step after a timeInterval
    // change the 50 to speed up or speed down. 
    if(millis() - timeStamp >= 500) // 100 is ms that takes the servo to move to the next step.
    {
      // -- This will update the time stamp to the last moment the servo moved.
      timeStamp = millis();
      servoPos += servoDir*step; // Servo moves 3 grades after 50ms 
      if(servoPos >= 180)
      {
        servoPos = 180;
        servoDir = -1;
      }
      if(servoPos <= 0)
      {
        servoPos = 0;
        servoDir = 1;
      }
      Servo1.write(servoPos);
    }
  } else{
    if (servoPos != 0) 
    {
      servoPos = 0;
      servoDir = 1; 
      Servo1.write(0);
    }
    timeStamp = 0;
  }
}

// -- Joystick variables

void loop()
{
  // -- If switcher is high, rada will be on, if is low, it will be turned off.
  if(digitalRead(SWITCHER_RS) == LOW) 
  {
    servoMotionAndRadar(false, 3);
    lcd.setCursor(0, 0);
    lcd.print(F("Radar is OFF."));
  } else {
    servoMotionAndRadar(true, 3);  
    lcd.setCursor(0, 0);
    lcd.print(F("Angle: "));
    lcd.print(servoPos);
    lcd.print(F("     "));
  }
}

