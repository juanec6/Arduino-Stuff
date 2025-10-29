#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Servo.h>

// -------------------- BUTTONS --------------------
#define BUTTON_MENU 7
#define BUTTON_UP 6
#define BUTTON_DOWN 5
#define BUTTON_BACK 4
#define BUTTON_ENTER 3
#define RED_LED 9
#define GREEN_LED 13

// -------------------- LCD & RTC --------------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;
Servo myServo1;

// --- MENU OPTIONS ---
const int numOptions = 4;
String options[numOptions] = {
  "Alarma",
  "Temporizador",
  "Mover Servo",
  "Encender LED"  //General purpose, case can be updated to do other tasks
};

// -------------------- DEBOUNCE CONTROL --------------------

const long debounceDelay = 250;  // 200 milisegundos para detectar una pulsación única
unsigned long lastDebounceMenu = 0;
unsigned long lastDebounceUp = 0;
unsigned long lastDebounceDown = 0;
unsigned long lastDebounceBack = 0;
unsigned long lastDebounceEnter = 0;

// -------------------- PROGRAM VARIABLES--------------------
int currentOption = 0;
int menuLevel = 0;  // 0 = time and date, 1 = Menu, 2 = submenú

// --- TIME HH:MM:SS AND DATE DD/MM/YY VARIABLES
int hour;
int minutes;
int seconds;
int day;
int month;
int year;

// -- TIME AND DATE VARIABLES TO SET ALARM
int alarmHour;
int alarmMinutes;
int alarmSeconds;
int alarmDay;
int alarmMonth;
int alarmYear;

// -- Alarm navigation variables and 
bool alarmSave = false;

// -- Servo
int posI = 0;    // Servo initial position
int posF = 180;  // Servo final position
bool servoMoved = false;
int currentPos = 0;

// =========================================================
//                        SETUP
// =========================================================
void setup() {
  Serial.begin(9600);

  // --- Button Configuration ---
  pinMode(BUTTON_MENU, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_BACK, INPUT_PULLUP);
  pinMode(BUTTON_ENTER, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  myServo1.attach(11);

  // --- LCD INITIALIZATION ---
  lcd.init();
  lcd.backlight();

  // --- RTC INITIALIZATION ---
  if (rtc.begin() == false) {
    lcd.print("Error RTC");
    while (1)
      ;
  }
  // --- Welcome Message ---
  lcd.setCursor(0, 0);
  lcd.print("Hola gonorrea");
  lcd.setCursor(0, 1);
  lcd.print("Que mas pues?");
  delay(2000);

  // Just adjust once to adjust the compilation time.
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, HIGH);
}

// =========================================================
//                        LOOP
// =========================================================

void loop() {
  //TIME AND DATE
  DateTime now = rtc.now();
  hour         = now.hour();
  minutes      = now.minute();
  seconds      = now.second() + 8;
  year         = now.year();
  month        = now.month();
  day          = now.day();
  alarmHour    = now.hour();
  alarmMinutes = now.minute();
  alarmSeconds = now.second() + 8;
  alarmDay     = now.day();
  alarmMonth   = now.month();
  alarmYear    = now.year();

  // -- LCD NAVIGATION AND FUNCTION --
  switch (menuLevel) {
    // =====================================================
    // --- TIME AND DATE SHOWN AT INITIALIZATION ---
    case 0:
      {
        // --- Show Time ---
        lcd.setCursor(0, 0);
        lcd.print("Time: ");
        if (hour < 10) lcd.print("0");
        lcd.print(hour);
        lcd.print(":");
        if (minutes < 10) lcd.print("0");
        lcd.print(minutes);
        lcd.print(":");
        if (seconds < 10) lcd.print("0");
        lcd.print(seconds);
        // --- Show Date ---
        lcd.setCursor(0, 1);
        lcd.print("Date: ");
        if (day < 10) lcd.print("0");
        lcd.print(day);
        lcd.print("/");
        if (month < 10) lcd.print("0");
        lcd.print(month);
        lcd.print("/");
        lcd.print(year);

        // --- Go to Menu ---
        if (digitalRead(BUTTON_MENU) == LOW) {
          if (millis() - lastDebounceMenu > debounceDelay) {
            lcd.clear();
            menuLevel = 1;
            lastDebounceMenu = millis();
          }
        }
        break;
      }

    // =====================================================
    // --- MENU NAVIGATION ---
    // =====================================================
    case 1:
      {
        lcd.setCursor(0, 0);
        lcd.print("Menu:           ");
        lcd.setCursor(0, 1);
        lcd.print("> ");
        lcd.print(options[currentOption]);
        lcd.print("               ");

        // --- DOWN button ---
        if (digitalRead(BUTTON_DOWN) == LOW) {
          if (millis() - lastDebounceDown > debounceDelay) {
            currentOption++;
            if (currentOption >= numOptions) currentOption = 0;
            lastDebounceDown = millis();
          }
        }

        // --- UP button ---
        if (digitalRead(BUTTON_UP) == LOW) {
          if (millis() - lastDebounceUp > debounceDelay) {
            currentOption--;
            if (currentOption < 0) currentOption = numOptions - 1;
            lastDebounceUp = millis();
          }
        }

        // --- ENTER button ---
        if (digitalRead(BUTTON_ENTER) == LOW) {
          if (millis() - lastDebounceEnter > debounceDelay) {
            lcd.clear();
            menuLevel = currentOption + 2;
            lastDebounceEnter = millis();
          }
        }

        // --- BACK button ---
        if (digitalRead(BUTTON_BACK) == LOW) {
          if (millis() - lastDebounceBack > debounceDelay) {
            lcd.clear();
            menuLevel = 0;
            currentOption = 0;
            lastDebounceBack = millis();
          }
        }
        break;
      }

    // =====================================================
    // --- ALARM ---
    // =====================================================
    case 2:
      { // -- First We show the actual time and date -- 
        // --- Show Time ---
        lcd.setCursor(0, 0);
        lcd.print("Alarm: ");
        if (alarmHour < 10) lcd.print("0");
        lcd.print(alarmHour);
        lcd.print(":");
        if (alarmMinutes < 10) lcd.print("0");
        lcd.print(alarmMinutes);
        lcd.print(":");
        if (alarmSeconds < 10) lcd.print("0");
        lcd.print(alarmSeconds);
        // --- Show Date ---
        lcd.setCursor(0, 1);
        lcd.print("Set: ");
        if (alarmDay < 10) lcd.print("0");
        lcd.print(day);
        lcd.print("/");
        if (alarmMonth < 10) lcd.print("0");
        lcd.print(alarmMonth);
        lcd.print("/");
        lcd.print(alarmYear);

        if (digitalRead(BUTTON_BACK) == LOW) {
          if (millis() - lastDebounceBack > debounceDelay) {
            lcd.clear();
            menuLevel = 1;
            lastDebounceBack = millis();
          }
        }
        break;
      }
      //    
    // =====================================================
    // --- TIMER ---
    // =====================================================
    case 3:
      {
        lcd.setCursor(0, 0);
        lcd.print("Soy timer");

        if (digitalRead(BUTTON_BACK) == LOW) {
          if (millis() - lastDebounceBack > debounceDelay) {
            lcd.clear();
            menuLevel = 1;
            lastDebounceBack = millis();
          }
        }
        break;
      }

    // =====================================================
    // --- SERVO MANAGEMENT ---
    // =====================================================
    case 4:
      {
        // Inicialización al entrar
        if (!servoMoved) {
            currentPos = posI;        // empieza desde la posición inicial
            myServo1.write(currentPos);
            servoMoved = true;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Servo Control");
        }

        // Mostrar posición
        lcd.setCursor(0, 1);
        lcd.print("Pos: ");
        lcd.print(currentPos);
        lcd.print("   "); // limpia dígitos anteriores

        // Botón UP
        if (digitalRead(BUTTON_UP) == LOW) {
            if (millis() - lastDebounceUp > debounceDelay) {
                currentPos++;
                if (currentPos > posF) currentPos = posF;
                myServo1.write(currentPos);
                lastDebounceUp = millis();
            }
        }

        // Botón DOWN
        if (digitalRead(BUTTON_DOWN) == LOW) {
            if (millis() - lastDebounceDown > debounceDelay) {
                currentPos--;
                if (currentPos < posI) currentPos = posI;
                myServo1.write(currentPos);
                lastDebounceDown = millis();
            }
        }

        // Botón BACK
        if (digitalRead(BUTTON_BACK) == LOW) {
            if (millis() - lastDebounceBack > debounceDelay) {
                lcd.clear();
                menuLevel = 1;
                servoMoved = false;  // reset para próxima vez
                lastDebounceBack = millis();
            }
        }
        break;
      }


    // =====================================================
    // --- LED MANAGEMENT ---
    // =====================================================
    case 5:
      {
        bool up = (digitalRead(BUTTON_UP) == LOW);
        bool down = (digitalRead(BUTTON_DOWN) == LOW);

        // Ambos presionados → Enciende ambos LEDs
        if (up && down) {
          digitalWrite(RED_LED, HIGH);
          digitalWrite(GREEN_LED, HIGH);
          lcd.setCursor(0, 0);
          lcd.print("> Green: ON     ");
          lcd.setCursor(0, 1);
          lcd.print("> Red: ON       ");
        }
        // Solo DOWN → Verde
        else if (down) {
          digitalWrite(GREEN_LED, HIGH);
          digitalWrite(RED_LED, LOW);
          lcd.setCursor(0, 0);
          lcd.print("> Green: ON     ");
          lcd.setCursor(0, 1);
          lcd.print("> Red: OFF      ");
        }
        // Solo UP → Rojo
        else if (up) {
          digitalWrite(RED_LED, HIGH);
          digitalWrite(GREEN_LED, LOW);
          lcd.setCursor(0, 0);
          lcd.print("> Green: OFF    ");
          lcd.setCursor(0, 1);
          lcd.print("> Red: ON       ");
        }
        // Ninguno presionado → texto normal
        else {
          digitalWrite(RED_LED, LOW);
          digitalWrite(GREEN_LED, LOW);
          lcd.setCursor(0, 0);
          lcd.print("> Green: OFF    ");
          lcd.setCursor(0, 1);
          lcd.print("> Red: OFF      ");
        }

        // Volver al menú
        if (digitalRead(BUTTON_BACK) == LOW) {
          lcd.clear();
          digitalWrite(GREEN_LED, HIGH);
          digitalWrite(RED_LED, HIGH);
          menuLevel = 1;
        }
        break;
      }
  }
}
