#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

// Arduino spots for buttons to control lcd
#define BUTTON_MENU 6
#define BUTTON_ENTER 5
#define BUTTON_BACK 4
#define BUTTON_UP 3
#define BUTTON_DOWN 2

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

// Thesecan be define as variables as well to work easy
int enter = BUTTON_ENTER;
int back = BUTTON_BACK; 
int menu = BUTTON_MENU;
int up = BUTTON_UP;
int down = BUTTON_DOWN;

//- Menu
const int numOptions = 4;
String options[numOptions] = {
  "Alarma",
  "Temporizador",
  "Activar Servo",
  "Salir"
};

void setup() {
  lcd.print("HOLA GONORREA!!!");
  delay(5000);
  // le dice al Arduino que esos pines son entradas digitales 
  // (para leer botones), pero con la resistencia interna de pull-up activada
  pinMode(enter, INPUT_PULLUP);
  pinMode(back, INPUT_PULLUP);
  pinMode(menu, INPUT_PULLUP);
  pinMode(up, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  if (!rtc.begin()) {
    lcd.print("Error RTC");
    while (1);
  }

  // Solo la primera vez: ajustar al tiempo de compilación
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

//Variables to navigate the menu 
int currentOption = 0;
int menuLevel = 0; //0 = time and date, 1 = Menu, 2 = submenú
void loop() {
  // --- DEBUG ---
  if (digitalRead(down) == LOW) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DOWN pressed!");
    delay(500);
  }

  // I'm going to use cases to nagivate and do changes
  switch(menuLevel) {
    //Case 0 will show time and date
    case 0: {
      DateTime now = rtc.now(); // ← ESTA LÍNEA FALTABA

      lcd.setCursor(0, 0);
      lcd.print("Time: ");
      // Mostrar hora HH:MM:SS
      if (now.hour() < 10) lcd.print("0");
      lcd.print(now.hour());
      lcd.print(":");
      if (now.minute() < 10) lcd.print("0");
      lcd.print(now.minute());
      lcd.print(":");
      if (now.second() < 10) lcd.print("0");
      lcd.print(now.second());

      // Mostrar fecha DD/MM/AAAA
      lcd.setCursor(0, 1);
      lcd.print("Date: ");
      if (now.day() < 10) lcd.print("0");
      lcd.print(now.day());
      lcd.print("/");
      if (now.month() < 10) lcd.print("0");
      lcd.print(now.month());
      lcd.print("/");
      lcd.print(now.year());
      delay(1000); // actualizar cada segundo

      // Now I can crate a condition that if the customer push the menu button,
      // it will go to the menu page:
      if(digitalRead(menu) == LOW){
        delay(200);
        lcd.clear();
        menuLevel = 1; // Pricipal menu
      }
      break;
    }
    
    // Menu display
    case 1: {
      lcd.setCursor(0, 0);
      lcd.print("Menu principal");
      lcd.setCursor(0, 1);
      lcd.print("> ");
      lcd.print(options[currentOption]);

      // Navegar hacia abajo
      if (digitalRead(down) == LOW) {
        delay(200);
        currentOption++;
        if (currentOption >= numOptions) currentOption = 0;
        lcd.clear();
      }

      // Navegar hacia arriba
      if (digitalRead(up) == LOW) {
        delay(200);
        currentOption--;
        if (currentOption < 0) currentOption = numOptions - 1;
        lcd.clear();
      }

      // Volver al reloj
      if (digitalRead(back) == LOW) {
        delay(200);
        lcd.clear();
        menuLevel--;
      }
      break;
    }
  }
}
