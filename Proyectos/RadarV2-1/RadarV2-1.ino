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
#define trigPin 8
#define echoPin 7

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

  // -- Sonar initialization
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

// -- FUNCTIONS AND VARIABLES OF COMPONENTS
// -- Servo and radar variables
int16_t servoPos = 0;
int8_t servoDir = 1;
unsigned long timeStamp = 0;// This is done to ensure the servo moves each 500 ms
unsigned long sonarTimeStamp = 0;// This is done to ensure sonar measures by 40 ms
float distance;
void servoMotionAndRadar(bool ON, int8_t step) // If ON is True, Servo and radar will work, if false, it wont. 
{
  unsigned long waveDuration;
  ON = digitalRead(SWITCHER_RS);
  if(ON == true) // Turn on servo. 
  {
    // -- Sonar data acquisition

    // we don't want the servo to move like crazy, so it makes a step after a timeInterval
    // change the 50 to speed up or speed down. 
    if(millis() - sonarTimeStamp >= 40 && millis() - timeStamp >= 500) // 100 is ms that takes the servo to move to the next step.
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
      sonarTimeStamp = millis();
      digitalWrite(trigPin, LOW); 
      delayMicroseconds(2); // --stabilizes pin
      digitalWrite(trigPin, HIGH); // -- Shoots sonar
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW); // -- Shoot ends.
      
      waveDuration = pulseIn(echoPin, HIGH);
      distance = (waveDuration*0.0343)/2; // --  speed of sound is in cm/microsecoonds and divided by two because is time going and coming back to the triger.
    }
  } else{
    if (ON == false) 
    {
      servoPos = 0;
      servoDir = 1;
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      waveDuration = pulseIn(echoPin, HIGH);
      distance = (waveDuration*0.0343)/2;
    }
    timeStamp = 0;
    Servo1.write(0);
  }
}


void loop()
{
  servoMotionAndRadar(LOW, 3);
  lcd.setCursor(0,0);
  lcd.print(F("Dist: "));
  lcd.print(F("    ")); // 4 espacios
  lcd.setCursor(6,0);
  lcd.print(distance);
  lcd.setCursor(12,0);
  lcd.print(" cm");

  // Línea 1
  lcd.setCursor(0,1);
  lcd.print(F("Angle: "));
  lcd.print(F("    "));
  lcd.setCursor(7,1);
  lcd.print(servoPos);
  lcd.setCursor(12,1);  // limpia la línea donde va el número
  lcd.print(F(" Grds"));
}

