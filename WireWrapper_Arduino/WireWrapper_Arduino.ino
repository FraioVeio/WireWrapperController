#include <LiquidCrystal.h>
#include <EEPROM.h>

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

#define DEBOUNCE_US 150000


/*
 * EEPROM Addresses
 * 0    - How many programs
 * 1    - First program length
 * 2    - First program type 0=angle 1=time
 * 3,6  - First instruction
 * 7,10 - Second instruction
 * ...
 * x    - Second program length
 * x+1  - Second program type
 * ...
 */

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
// Variabili per eventi dei pulsanti
bool b1p, b2p, b3p, b4p;
long b1m, b2m, b3m, b4m;

void setup() {
  b1p = b2p = b3p = b4p;
  
  lcd.begin(16, 2); // Colonne e righe LCD

  Serial.begin(9600);

  attachInterrupt(digitalPinToInterrupt(ENCODER), encoderInterrupt, CHANGE);

  pinMode(RELAY, OUTPUT);
  pinMode(BT_1, INPUT);
  pinMode(BT_2, INPUT);
  pinMode(BT_3, INPUT);
  pinMode(BT_4, INPUT);
  pinMode(ENCODER, INPUT);
  
}

int bc = 0;

void b1_press() {
  
}

void b2_press() {
  
}

void b3_press() {
  
}

void b4_press() {
  
}


void loop() {
  // Gestione eventi pulsanti
  if(digitalRead(BT_1) && !b1p) {
    b1p = true;
    b1m = micros();

    b1_press();
  }
  if(!digitalRead(BT_1) && b1p) {
    if(micros()-b1m >= DEBOUNCE_US)
      b1p = false;
  }
  if(digitalRead(BT_2) && !b2p) {
    b2p = true;
    b2m = micros();

    b2_press();
  }
  if(!digitalRead(BT_2) && b2p) {
    if(micros()-b2m >= DEBOUNCE_US)
      b2p = false;
  }
  if(digitalRead(BT_3) && !b3p) {
    b3p = true;
    b3m = micros();

    b3_press();
  }
  if(!digitalRead(BT_3) && b3p) {
    if(micros()-b3m >= DEBOUNCE_US)
      b3p = false;
  }
  if(digitalRead(BT_4) && !b4p) {
    b4p = true;
    b4m = micros();

    b4_press();
  }
  if(!digitalRead(BT_4) && b4p) {
    if(micros()-b4m >= DEBOUNCE_US)
      b4p = false;
  }
  // Fine eventi pulsanti

  
}

void encoderInterrupt() {
  
}
