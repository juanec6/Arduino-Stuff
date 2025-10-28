#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DS1302.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
DS1302 rtc(8, 7, 6);

#define VRX A0
#define VRY A1
#define SW  2

int state = 0;
bool pressed = false;

void setup() {
  pinMode(SW, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  rtc.halt(false);
  rtc.writeProtect(false);
  // rtc.setTime(12, 0, 0); // hh, mm, ss (solo una vez si quieres fijar hora)
}

void loop() {
  int x = analogRead(VRX);
  int y = analogRead(VRY);
  bool swState = !digitalRead(SW);

  if (y > 800) state = (state + 1) % 3;     // abajo
  if (y < 200) state = (state - 1 + 3) % 3; // arriba

  if (swState && !pressed) {
    pressed = true;
    // acción según el menú actual
  } else if (!swState) pressed = false;

  showMenu();
  delay(200);
}

void showMenu() {
  lcd.clear();
  switch(state) {
    case 0:
      Time t = rtc.getTime();
      lcd.setCursor(0,0); lcd.print("Hora actual:");
      lcd.setCursor(0,1);
      lcd.print(String(t.hour) + ":" + String(t.min) + ":" + String(t.sec));
      break;
    case 1:
      lcd.print("Cambiar hora");
      break;
    case 2:
      lcd.print("Configurar alarma");
      break;
  }
}
