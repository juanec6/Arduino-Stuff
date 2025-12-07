#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Servo.h>

// -- COMPONENTS --
#define BUTTON_MENU 7
#define BUTTON_UP 6
#define BUTTON_DOWN 5
#define BUTTON_BACK 4
#define BUTTON_ENTER 3
#define RED_LED 8
#define GREEN_LED 13
#define trigPin 9
#define echoPin 10

// -- LCD & RTC --
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;
Servo Servo1;

// -- MENU OPTIONS --
const uint8_t numOptions = 5;
const char* menu[numOptions] = {
  "Radar",
  "Logs",
  "Radar Settings",
  "Time Settings",
  "Alarm"
};

// -- DEBOUNCE CONTROL --
const uint16_t debounceDelay = 300;
uint16_t lastDebounce = 0;

// -- MENU SET --
int8_t currentOption = 0;
int8_t menuLevel = 0;

// -- TIME AND DATE VARIABLES
uint8_t hour;
uint8_t minutes;
uint8_t seconds;
uint8_t day;
uint8_t month;
uint16_t year;

// -- SERVO SET --
uint8_t servoPos = 0;
int8_t servoDir = 1; // 1 right, -1 left
bool servoStopped = false;

// -- LAST DETECTION --
uint16_t lastDistance = 999;
uint8_t lastAngle = 0;
uint8_t lastHour = 0;
uint8_t lastMinute = 0;
uint8_t lastSecond = 0;
bool detectionSaved = false;

// -- Servo Motion
void servoMotion() {
  if (servoStopped) return; 
  static unsigned long lastMove = 0;

  if (millis() - lastMove >= 1) {   // Servo velocity
    lastMove = millis();
    servoPos += servoDir;
    if (servoPos >= 180) servoDir = -1; // Servo goes from 1° to 180°
    if (servoPos <= 0)   servoDir = 1;
    Servo1.write(servoPos);
  }
}

// -- 
void setup()
{
  Serial.begin(9600);
  // --- Button configuration ---
  pinMode(BUTTON_MENU, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_BACK, INPUT_PULLUP);
  pinMode(BUTTON_ENTER, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  // --- Servo Attach ---
  Servo1.attach(11);

  // -- LCD INITIALIZATION --
  lcd.init();
  lcd.backlight();

  // -- RTC INITIALIZATION --
  if(rtc.begin() == false)
  {
    lcd.print(F("Error RTC"));
    while(1);
  }
  // -- RADAR --
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, HIGH);
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() 
{
  // time set
  DateTime now = rtc.now();
  hour         = now.hour();
  minutes      = now.minute();
  seconds      = now.second();
  year         = now.year();
  month        = now.month();
  day          = now.day();

  // -- LCD NAVIGATION --
  switch(menuLevel)
  {
    case 0:
    {
      // DISPLAY TIME
      lcd.setCursor(0, 0);
      lcd.print(F("Time: "));
      if (hour < 10) lcd.print(F("0"));
      lcd.print(hour);
      lcd.print(F(":"));
      if (minutes < 10) lcd.print(F("0"));
      lcd.print(minutes);
      lcd.print(F(":"));
      if (seconds < 10) lcd.print(F("0"));
      lcd.print(seconds);
      lcd.print(F("   "));

      // DISPLAY DATE
      lcd.setCursor(0, 1);
      lcd.print(F("Date: "));
      if (day < 10) lcd.print(F("0"));
      lcd.print(day);
      lcd.print(F("/"));
      if (month < 10) lcd.print(F("0"));
      lcd.print(month);
      lcd.print(F("/"));
      lcd.print(year);
      lcd.print(F(" "));

      // GO TO MENU
      if (digitalRead(BUTTON_MENU) == LOW)
      {
        if(millis() - lastDebounce > debounceDelay)
        {
          lastDebounce = millis();
          menuLevel++;
          lcd.clear();
        }
      }
      break;
    }

    // -- MENU NAVIGATION
    case 1:
    {
      int8_t upper = currentOption;
      int8_t lower = (currentOption + 1) % numOptions;
      lcd.setCursor(0, 0);
      lcd.print(F("> "));
      lcd.print(menu[upper]);
      lcd.print(F("                "));

      lcd.setCursor(0, 1);
      lcd.print(F("  "));
      lcd.print(menu[lower]);
      lcd.print(F("                "));

      // DOWN BUTTON
      if(digitalRead(BUTTON_DOWN) == LOW)
      {
        if(millis() - lastDebounce > debounceDelay)
        {
          currentOption++;
          if(currentOption >= numOptions) currentOption = 0;
          lastDebounce = millis();
          lcd.clear();
        }
      }

      // UP BUTTON
      if(digitalRead(BUTTON_UP) == LOW)
      {
        if(millis() - lastDebounce> debounceDelay)
        {
          currentOption--;
          if(currentOption < 0) currentOption = numOptions - 1;
          lastDebounce = millis();
          lcd.clear();
        }
      }

      // ENTER BUTTON
      if(digitalRead(BUTTON_ENTER) == LOW)
      {
        if(millis() - lastDebounce > debounceDelay)
        {
          lcd.clear();
          menuLevel = currentOption + 2;
          lastDebounce = millis();
        }
      }

      // BACK BUTTON
      if(digitalRead(BUTTON_BACK) == LOW)
      {
        if(millis() - lastDebounce > debounceDelay)
        {
          lcd.clear();
          menuLevel--;
          currentOption = 0;
          lastDebounce = millis();
          
        }
      }
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, HIGH);
      break;
    }

    // -- Radar
    case 2:
    {
      if (menuLevel == 2) servoMotion();
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      unsigned long duration = pulseIn(echoPin, HIGH, 28000);
      uint16_t distance = (duration * 0.0343) / 2;

      // Línea 0
      lcd.setCursor(0,0);
      lcd.print(F("Dist: "));
      lcd.print(F("    ")); // 4 espacios
      lcd.setCursor(6,0);
      lcd.print(distance);
      lcd.setCursor(11,0);
      lcd.print(" cm");

      // Línea 1
      lcd.setCursor(0,1);
      lcd.print(F("Angle: "));
      lcd.print(F("    "));
      lcd.setCursor(7,1);
      lcd.print(servoPos);
      lcd.setCursor(11,1);  // limpia la línea donde va el número
      lcd.print(F(" Grds"));
      if(distance < 35)
      {
        servoStopped = true;
        Servo1.write(servoPos);
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(RED_LED, HIGH);
        // Save detection only once per event
        lastDistance = distance;
        lastAngle = servoPos;
        lastHour = hour;
        lastMinute = minutes;
        lastSecond = seconds;
        detectionSaved = true;
      } 
      else 
      {
        servoStopped = false;
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(RED_LED, LOW);
      }


      if(digitalRead(BUTTON_BACK) == LOW){
        if(millis() - lastDebounce > debounceDelay){
          lcd.clear();
          menuLevel = 1;
          lastDebounce = millis();
        }
      }
      break;
    }
    
    // Real time logs
    case 3:
    {
      servoMotion();
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      unsigned long duration = pulseIn(echoPin, HIGH, 28000);
      uint16_t distance = duration*(0.0343)/2;

      lcd.setCursor(0, 0);
      if(detectionSaved == true)
      {
        lcd.print(F("Time: "));
        if(lastHour < 10) lcd.print(F("0"));
        lcd.print(lastHour);
        lcd.print(F(":"));
        if(lastMinute < 10) lcd.print(F("0"));
        lcd.print(lastMinute);
        lcd.print(F(":"));
        if(lastSecond < 10) lcd.print(F("0"));
        lcd.print(lastSecond);
        lcd.setCursor(0, 1);
        lcd.print(F("A: "));
        lcd.print(lastAngle);
        lcd.print(F(" "));
        lcd.print(F(" D: "));
        lcd.print(lastDistance);
        lcd.print(F("   "));
        
      } else
      {
        lcd.print(F("No detections."));
        lcd.setCursor(0, 1);
        lcd.print(F("Dist < 35 cm"));
      }

      if(distance < 35)
      {
        servoStopped = true;
        Servo1.write(servoPos);
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(RED_LED, HIGH);
        // Save detection only once per event
        lastDistance = distance;
        lastAngle = servoPos;
        lastHour = hour;
        lastMinute = minutes;
        lastSecond = seconds;
        detectionSaved = true;
      } 
      else 
      {
        servoStopped = false;
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(RED_LED, LOW);
      }
      
      Serial.print("Dst: "); 
      Serial.print(distance);
      Serial.print(", Ang: "); 
      Serial.print(servoPos);

      if(distance < 35)
      {
        Serial.print(", Movement Detected at ");
        if(lastHour < 10) Serial.print("0");
        Serial.print(lastHour);
        Serial.print(":");
        if(lastMinute < 10) Serial.print("0");
        Serial.print(lastMinute);
        Serial.print(":");
        if(lastSecond < 10) Serial.print("0");
        Serial.println(lastSecond);
      } 
      else 
      {
        Serial.println(", No Movement Detected");
      }
      
      if(digitalRead(BUTTON_BACK) == LOW) 
      {
        if(millis() - lastDebounce > debounceDelay)
        {
          menuLevel = 1;
          lastDebounce = millis();
          lcd.clear();
        }
      }
      break;
    }
    
    // -- Radar Settings
    case 4:
    {
      lcd.setCursor(0, 0);
      lcd.print(F("Manual Radar"));
      if(digitalRead(BUTTON_BACK) == LOW) 
      {
        if(millis() - lastDebounce > debounceDelay)
        {
          menuLevel = 1;
          lastDebounce = millis();
          lcd.clear();
        }
      }
      break;
    }

    // -- Time Settings
    case 5:
    {
      lcd.setCursor(0, 0);
      lcd.print(F("Time Settings goes here"));
      if(digitalRead(BUTTON_BACK) == LOW) 
      {
        if(millis() - lastDebounce > debounceDelay)
        {
          menuLevel = 1;
          lastDebounce = millis();
          lcd.clear();
        }
      }
      break;
    }

    case 6:
    {
      lcd.setCursor(0, 0);
      lcd.print(F("Alarm goes here"));
      if(digitalRead(BUTTON_BACK) == LOW) 
      {
        if(millis() - lastDebounce > debounceDelay)
        {
          menuLevel = 1;
          lastDebounce = millis();
          lcd.clear();
        }
      }
      break;
    }
  }
}
