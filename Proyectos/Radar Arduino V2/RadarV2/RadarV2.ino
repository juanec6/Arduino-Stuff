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
#define DEADZONE 80

// -- CLASSES
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;
Servo Servo1;

// -- MENU OPTIONS
const uint8_t numOptions = 5;
const char* menu[numOptions] = {
  "Radar",
  "Logs",
  "Manual Op",
  "Time Settings",
  "Alarm"
};

// -- DEBOUNCE CONTROL
const uint32_t debounceDelay = 500;
uint16_t lastDebounceBack = 0;
uint16_t lastDebounceEnter = 0;

// -- JOYSTICK VARIABLES
int16_t joyX;
int16_t joyY;
int8_t joyDirY = 0;    // -1, 0, +1
int8_t lastJoyDirY = 0;
int8_t joyDirX = 0;    // -1, 0, +1
int8_t lastJoyDirX = 0;

// -- MENU VARIABLES
int menuLevel = 0;
int currentOption = 0;

// -- SERVO MOTION
uint8_t servoPos = 90;
int8_t servoDir = 1;
bool servoStopped = false;

// SETUP 
void setup() {
  Serial.begin(9600);
  // -- lcd set up
  lcd.init();
  lcd.backlight();

  // -- rtc set up
  if(rtc.begin() == false)
  {
    lcd.print(F("Error RTC"));
    while(1);
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  // -- leds set up
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, HIGH);

  // -- Servo set up
  Servo1.attach(SERVO_R);
  Servo1.write(servoPos);

  // -- Digital outputs
  pinMode(ENTER_BUTTON, INPUT_PULLUP);
  pinMode(BACK_BUTTON, INPUT_PULLUP);
}

// -- LOOP
void loop() {
  // -- Joystick settings
  joyX = analogRead(JOY_X) - 512;
  joyY = (analogRead(JOY_Y) - 512)*(-1);

  // -- X-axis Joystick centering and position states
  if(abs(joyX) < DEADZONE) joyX = 0;
  if(joyX > 0)       joyDirX = 1;
  else if(joyX < 0)  joyDirX = -1;
  else               joyDirX = 0;

  // -- Y-axis Joystick dentering and states
  if(abs(joyY) < DEADZONE) joyY = 0;
  if(joyY > 0)       joyDirY = 1;
  else if(joyY < 0)  joyDirY = -1;
  else               joyDirY = 0;

  // -- LCD navigation
  switch(menuLevel)
  { // -- Time and date
    case 0:
    {
      // -- Display time
      DateTime now = rtc.now();
      lcd.setCursor(0, 0);
      lcd.print(F("Time: "));
      if (now.hour() < 10) lcd.print(F("0"));
      lcd.print(now.hour());
      lcd.print(F(":"));
      if (now.minute() < 10) lcd.print(F("0"));
      lcd.print(now.minute());
      lcd.print(F(":"));
      if (now.second() < 10) lcd.print(F("0"));
      lcd.print(now.second());
      lcd.print(F("   "));

      // -- Display date
      lcd.setCursor(0, 1);
      lcd.print(F("Date: "));
      if (now.day() < 10) lcd.print(F("0"));
      lcd.print(now.day());
      lcd.print(F("/"));
      if (now.month() < 10) lcd.print(F("0"));
      lcd.print(now.month());
      lcd.print(F("/"));
      if (now.year() < 10) lcd.print(F("0"));
      lcd.print(now.year());
      lcd.print(F("   "));

      // -- Access menu
      if(digitalRead(ENTER_BUTTON) == LOW && millis() - lastDebounceEnter > debounceDelay)
      {
        lastDebounceEnter = millis();
        menuLevel = 1;
        lcd.clear();
      }
      break;
    } // Case 0 end

    // -- Menu navigation
    case 1:
    {
      int upper = currentOption;
      int lower = (currentOption + 1) % numOptions;
      lcd.setCursor(0, 0);
      lcd.print(F("> "));
      lcd.print(menu[upper]);
      lcd.setCursor(0, 1);
      lcd.print(F("  "));
      lcd.print(menu[lower]);

      // -- Joystick navigation control
      // Down
      if(joyDirY == -1 && lastJoyDirY == 0){
        currentOption++;
        if(currentOption >= numOptions) currentOption = 0;
        lcd.clear();
      }

      // Up
      if(joyDirY == 1 && lastJoyDirY == 0){
        lcd.clear();
        currentOption--;
        if(currentOption < 0) currentOption = numOptions - 1;
      }

      // -- Enter Button
      if(digitalRead(ENTER_BUTTON) == LOW && millis() - lastDebounceEnter > debounceDelay)
      {
        lcd.clear();
        lastDebounceEnter = millis();
        menuLevel = currentOption + 2;
      }

      // -- Back to home screen
      if(digitalRead(BACK_BUTTON) == LOW && millis() - lastDebounceBack > debounceDelay)
      {
        lcd.clear();
        lastDebounceBack = millis();
        menuLevel = 0;
        currentOption = 0;
      }
      break;
    } // Case 1 end

    // -- Radar Monitoring
    case 2: 
    {
      servoMotion();

      if(digitalRead(BACK_BUTTON) == LOW && millis() - lastDebounceBack > debounceDelay)
      {
        lcd.clear();
        lastDebounceBack = millis();
        menuLevel = 1;
        servoPos = 90;
        currentOption = 0;
      }
      break;
    } // Case 2 end

    // -- Radar detections
    case 3:
    {
      lcd.setCursor(0, 0);
      lcd.print(F("Hola gonorrea"));

      // -- Back button
      if(digitalRead(BACK_BUTTON) == LOW && millis() - lastDebounceBack > debounceDelay)
      {
        lcd.clear();
        lastDebounceBack = millis();
        menuLevel = 1;
        servoPos = 90;
        currentOption = 0;
      }
      break;
    } // Case 3 end

    // -- Manual radar operation
    case 4:
    {
      static unsigned long lastManualMove = 0;
      int absJoy = abs(joyX);
      
      // -- Threshold
      if (absJoy > 100) 
      {
        // -- Velocity increment
        int interval = map(constrain(absJoy, 100, 512), 80, 512, 80, 3);

        // -- Not blocking Timer
        if (millis() - lastManualMove >= (unsigned long)interval) 
        {
          lastManualMove = millis();
          if (joyX > 100 && servoPos < 180) 
          {
            servoPos++;
          } 
          else if (joyX < -100 && servoPos > 0) 
          {
            servoPos--;
          }
          Servo1.write(servoPos);
        }
      }

      // -- Display info
      lcd.setCursor(0, 0);
      lcd.print(F("Angle: "));
      lcd.print(servoPos);
      lcd.print(F("      ")); 

      // -- Back button
      if(digitalRead(BACK_BUTTON) == LOW && millis() - lastDebounceBack > debounceDelay)
      {
        lcd.clear();
        lastDebounceBack = millis();
        menuLevel = 1;
        servoPos = 90;
        currentOption = 0;
        Servo1.write(servoPos);
      }
      break;
    }

    // -- Time Setting
    case 5:
    {
      lcd.setCursor(0, 0);
      lcd.print("Hola puto");

            // -- Back button
      if(digitalRead(BACK_BUTTON) == LOW && millis() - lastDebounceBack > debounceDelay)
      {
        lcd.clear();
        lastDebounceBack = millis();
        menuLevel = 1;
        currentOption = 0;
      }
      break;
    }

    // -- Alarm
    case 6:
    {
      lcd.setCursor(0, 0);
      lcd.print("Hola putisimo");

            // -- Back button
      if(digitalRead(BACK_BUTTON) == LOW && millis() - lastDebounceBack > debounceDelay)
      {
        lcd.clear();
        lastDebounceBack = millis();
        menuLevel = 1;
        currentOption = 0;
      }
      break;
    }
  } // Switch end
  lastJoyDirY = joyDirY;
}

// -- COMPONENTES FUNCTIONS
// -- Servo motion
void servoMotion()
{ 
  static unsigned long lastMove = 0;
  if(millis() - lastMove >= 50)
  {
    lastMove = millis();
    servoPos += servoDir;
    if(servoPos >= 180) servoDir = -1;
    if(servoPos <= 0) servoDir = 1;
    Servo1.write(servoPos);
  }
}
