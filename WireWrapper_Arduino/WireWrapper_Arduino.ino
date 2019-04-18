#include <LiquidCrystal.h>

#define BT_1 A0
#define BT_2 A1
#define BT_3 A2
#define BT_4 A3

#define ENCODER 7

#define RELAY 6

#define RS 12
#define EN 11
#define D4 5
#define D5 4
#define D6 3
#define D7 2


LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

void setup() {
  lcd.begin(16, 2); // Colonne e righe LCD
  lcd.print("Hello, world!");
}

void loop() {
  
}
