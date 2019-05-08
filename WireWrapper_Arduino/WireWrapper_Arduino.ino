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

#define DEBOUNCE_US (long)200*1000


/*
 * EEPROM Addresses
 * 
 * Programmi lunghi 50 byte, totale 20 programmi
 * 
 * byte 0 = 0 0xS<lunghezza> dove S vale 0 se è controllato in tempo e 1 se in angoli;
 * byte 1...49 angolo o tempo
 */

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
// Variabili per eventi dei pulsanti
bool b1p, b2p, b3p, b4p;
long b1m, b2m, b3m, b4m;

//Variabili menu
bool refreshMenu;
bool insideProgram;
int programId, totPrograms, programSize;

byte selectedFirstByte;

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
  
  // Inizializzo menu
  refreshMenu = insideProgram = false;
  programId = 0;
  totPrograms = 20;
  printMenu();
}

int bc = 0;

void b1_press() {
  if(!insideProgram) {
    insideProgram = true;
    refreshMenu = true;
  }
}

void b2_press() {
  
}

void b3_press() {
  if(!insideProgram) {
    // Menu-
    programId --;
    if(programId <0 ) {
      programId = totPrograms-1;
    }
    refreshMenu = true;
  }
}

void b4_press() {
  if(!insideProgram) {
    // Menu+
    programId ++;
    if(programId >= totPrograms) {
      programId = 0;
    }
    refreshMenu = true;
  } else {
    // TODO: Metti i controlli se è ancora attivo
    insideProgram = false;
    refreshMenu = true;
  }
}

boolean a = false;
long t = 0;
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

  // Menu
  if(refreshMenu)
    printMenu();
  
  if(Serial.available()) {
    byte cmd = Serial.read();
    if(cmd == 65) { // Read EEPROM
      lcd.clear();
      lcd.print("Reading EEPROM..");
      printMenu();
      int size = EEPROM.length();
      Serial.write((char*)(&size), sizeof(int));
      for(int i=0;i<EEPROM.length();i++) {
        Serial.write(EEPROM.read(i));
      }
    }

    if(cmd == 66) { // Test Connection
      lcd.clear();
      lcd.print("Test connection");
      while(!Serial.available());
      Serial.write(Serial.read());
      
      printMenu();
    }

    if(cmd == 67) { // Read a program
      lcd.clear();
      lcd.print("Reading program...");
      while(!Serial.available());
      int id = Serial.read();
      for(int i=id*50;i<(id+1)*50;i++) {
        Serial.write(EEPROM.read(i));
      }
      
      printMenu();
    }

    if(cmd == 68) { // Write a program
      lcd.clear();
      lcd.print("Writing program...");
      while(!Serial.available());
      byte id = Serial.read();
      for(int i=id*50;i<(id+1)*50;i++) {
        while(!Serial.available());
        byte b = Serial.read();
        if(EEPROM.read(i) != b)
          EEPROM.write(i, b);
      }
      
      printMenu();
    }
  }
}

void printMenu() {
  refreshMenu = false;
  lcd.clear();
  if(!insideProgram) {
    lcd.print("Programma ");
    lcd.print(programId+1);
    lcd.print("/");
    lcd.print(totPrograms);
    
    lcd.setCursor(0, 1);
    if(totPrograms != 0) {
      int count = 0;
      byte info = EEPROM.read(programId * 50);
      if(info == 0)
        lcd.print("_Vuoto_");
      else {
        if(programType(info))
          lcd.print("Angoli: ");
        else
          lcd.print("Tempi: ");
        programSize = a & 0b01111111;
        lcd.print(programSize);
      }
    }
  } else {
    // Inside program
    lcd.print("Programma ");
    lcd.print(programId + 1);
    if(programType(selectedFirstByte))
      lcd.print(" Ang");
    else
      lcd.print(" Time");
    
    lcd.setCursor(0, 1);
    lcd.print("Start Stop Exit");
  }
}

boolean programType(byte b) { // True angoli
  return (b & 0b10000000) != 0;
}

void formatEeprom() {
  for(int i=0;i<EEPROM.length();i++) {
    EEPROM.write(i, 0);
  }
}

void encoderInterrupt() {
  
}
