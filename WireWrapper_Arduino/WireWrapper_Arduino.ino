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
#define RELAY_TIME_ON 100000
#define PROGRAMSIZE 101  // 1 control + 25 comandi da 4 byte, 10 programmi totali

/*
 * EEPROM Addresses
 * 
 * Programmi lunghi PROGRAMSIZE byte, totale 1024/PROGRAMSIZE programmi
 * 
 * byte 0 = 0 0xS<lunghezza> dove S vale 0 se è controllato in tempo e 1 se in angoli;
 * byte 1...PROGRAMSIZE-1 angolo o tempo
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

// Program Variables
bool started = false;
bool type;
int cStep;
int nSteps;
long timeOld = -1;
float currentValue;
boolean relayOn;
long relayTime;

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
  totPrograms = EEPROM.length()/PROGRAMSIZE;
  printMenu();
}

int bc = 0;

void b1_press() {
  if(!insideProgram) {
    insideProgram = true;
    refreshMenu = true;
  } else {
    // Start
    started = true;
    timeOld = -1;
    byte info = EEPROM.read(programId*PROGRAMSIZE);
    type = programType(info);
    nSteps = (info & 0b01111111);
    cStep = 0;
    refreshMenu = true;
    currentValue = getCurrentProgramValue();
  }
}

void b2_press() {
  if(insideProgram) {
    started = false;
    relayOn = false;
    relayTime = -1;
    refreshMenu = true;
    digitalWrite(RELAY, LOW);
  } else {
    // RIMUOVIMI
    for(int i=0;i<programSize;i++) {
      Serial.println(eepromReadFloat(programId*PROGRAMSIZE+1 + i*4));
    }
  }
}

void b3_press() {
  if(!insideProgram) {
    // Menu-
    programId --;
    if(programId <0 ) {
      programId = totPrograms-1;
    }
    refreshMenu = true;
  } else {
    started = false;
    relayOn = false;
    relayTime = -1;
    refreshMenu = true;
    digitalWrite(RELAY, LOW);
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

  // Seriale
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
      Serial.write(PROGRAMSIZE);
      for(int i=id*PROGRAMSIZE;i<(id+1)*PROGRAMSIZE;i++) {
        Serial.write(EEPROM.read(i));
      }
      
      printMenu();
    }

    if(cmd == 68) { // Write a program
      lcd.clear();
      lcd.print("Writing program...");
      while(!Serial.available());
      byte id = Serial.read();  // Invia ID
      while(!Serial.available());
      byte first = (Serial.read()==0 ? 0b00000000 : 0b10000000);  // Invia tipo, poi lunghezza (/4 byte)
      while(!Serial.available());
      byte sz = Serial.read();
      first = first | sz;
      
      if(EEPROM.read(id*PROGRAMSIZE) != first)
        EEPROM.write(id*PROGRAMSIZE, first);
      
      
      for(int i=id*PROGRAMSIZE+1;i<id*PROGRAMSIZE+1+sz*4;i++) {
        while(!Serial.available());
        byte b = Serial.read(); // Invia le robe
        if(EEPROM.read(i) != b)
          EEPROM.write(i, b);
      }
      
      printMenu();
    }
  }

  // Gestione relay
  if(relayOn) {
    if(micros()-relayTime < RELAY_TIME_ON) {
      digitalWrite(RELAY, HIGH);
    } else {
      relayOn = false;
      digitalWrite(RELAY, LOW);
    }
  }

  // Gestione programma
  if(started) {
    long timeNew = micros();
    if(timeOld == -1)
      timeOld = timeNew;

    if(!type) {
      // Programma a tempo
      if((timeNew-timeOld)/1000000.0 >= currentValue) {
        cStep ++;
        if(cStep >= nSteps)
          cStep = 0;
        timeOld = micros();
        
        currentValue = getCurrentProgramValue();
        relayOn = true;
        relayTime = micros();
      }
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
      byte info = EEPROM.read(programId * PROGRAMSIZE);
      if(info == 0)
        lcd.print("_Vuoto_");
      else {
        if(programType(info))
          lcd.print("Angoli: ");
        else
          lcd.print("Tempi: ");
        programSize = (info & 0b01111111);
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
    if(started) {
      lcd.print("      Stop Exit");
    } else {
      lcd.print("Start      Exit");
    }
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

float getCurrentProgramValue() {
  return eepromReadFloat(programId*PROGRAMSIZE+1 + cStep*4);
}

float eepromReadFloat(int index) {
  float f;
  byte *fp = (byte*) &f;
  for(int i=0;i<4;i++) {
    fp[i] = EEPROM.read(index+i);
  }
}

void encoderInterrupt() {
  
}
