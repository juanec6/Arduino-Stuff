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
#define SERVO 11



// -------------------- LCD & RTC --------------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;
Servo myServo;

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

// -------------------- MENU VARIABLES --------------------
int currentOption = 0;
int menuLevel = 0;  // 0 = time and date, 1 = Menu, 2 = submenú

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
  lcd.setCursor(0, 1);
  lcd.print("Hola gonorrea");
  delay(500);

  // Just adjust once to adjust the compilation time.
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, HIGH);
}

// =========================================================
//                        LOOP
// =========================================================
// TIME AND DATE DD/MM/YY VARIABLES
int hour;
int minutes;
int seconds;
int day;
int month;
int year;


void loop() {
  //TIME AND DATE
  DateTime now = rtc.now();
  hour = now.hour();
  minutes = now.minute();
  seconds = now.second() + 8;
  day = now.day();
  month = now.month();
  year = now.year();

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
      {
        lcd.setCursor(0, 0);
        lcd.print("Soy alarma");

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
        lcd.setCursor(0, 0);
        lcd.print("Soy servito");

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
