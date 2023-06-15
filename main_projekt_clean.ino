/*

   All the resources for this project: https://www.hackster.io/Aritro
   Modified by Aritro Mukherjee


*/

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.


int  north = 0, south  = 0, east  = 0, west  = 0;
const int  maxRow = 4;
int data[maxRow][4];
int delayer = 0;
int row = 0;
int password[maxRow][4];


boolean lcd_acces_granted = false, opened = false;

int xPosition = 0;
int yPosition = 0;

//beeper pin
const int Beeper = 8;

//joystick pins
const int VRx = A0;
const int VRy = A1;

//RGB led pin
const int ledPinG = A2;    // the number of the LED pin
const int ledPinR = A3;    // the number of the LED pin

//buttons pins 
const int buttonEnterPin = 2;  // the number of the pushbutton pin

//debouncers for buttons
boolean debouncerEnter = false;

#define A 0
#define B 1
#define C 6
#define D 7
 
#define NUMBER_OF_STEPS_PER_REV 110


int dealyBetweenPulses = 1;
int i=0;

int SW_state = 0;
void setup()
{
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

  pinMode(VRx, INPUT);
  pinMode(VRy, INPUT);
  pinMode(Beeper, OUTPUT);

  pinMode(buttonEnterPin, INPUT_PULLUP);
  pinMode(ledPinR, OUTPUT);
  pinMode(ledPinG, OUTPUT);

  
  lcd.begin();
  lcd.noBacklight();
  lcd.noDisplay();
  
  
  digitalWrite(ledPinR, HIGH);
  
  //set password
 /* password[0][0] = 10;
  password[0][1] = 0;
  password[0][2] = 10;
  password[0][3] = 0;

  password[1][0] = 5;
  password[1][1] = 0;
  password[1][2] = 6;
  password[1][3] = 0;

  password[2][0] = 9;
  password[2][1] = 0;
  password[2][2] = 0;
  password[2][3] = 0;

  password[3][0] = 8;
  password[3][1] = 0;
  password[3][2] = 0;
  password[3][3] = 3;*/


  password[0][0] = 0;
  password[0][1] = 0;
  password[0][2] = 0;
  password[0][3] = 0;

  password[1][0] = 0;
  password[1][1] = 0;
  password[1][2] = 0;
  password[1][3] = 0;

  password[2][0] = 0;
  password[2][1] = 0;
  password[2][2] = 0;
  password[2][3] = 0;

  password[3][0] = 0;
  password[3][1] = 0;
  password[3][2] = 0;
  password[3][3] = 0;

  //clearData();



  pinMode(A,OUTPUT);
  pinMode(B,OUTPUT);
  pinMode(C,OUTPUT);
  pinMode(D,OUTPUT);
}
void loop() {
  card_reader();
  if (!lcd_acces_granted) {
    return;
  }
  if (delayer == 0) {
    joystick_direction_setter();
  }
  ++delayer;
  if (delayer > 10) {
    delayer = 0;
  }

  reset_button();
  enter_button();
  

}

void stepperWrite(int a,int b,int c,int d){
digitalWrite(A,a);
digitalWrite(B,b);
digitalWrite(C,c);
digitalWrite(D,d);
}

void onestep(){
stepperWrite(1,0,0,0);
delay(dealyBetweenPulses);
stepperWrite(1,1,0,0);
delay(dealyBetweenPulses);
stepperWrite(0,1,0,0);
delay(dealyBetweenPulses);
stepperWrite(0,1,1,0);
delay(dealyBetweenPulses);
stepperWrite(0,0,1,0);
delay(dealyBetweenPulses);
stepperWrite(0,0,1,1);
delay(dealyBetweenPulses);
stepperWrite(0,0,0,1);
delay(dealyBetweenPulses);
stepperWrite(1,0,0,1);
delay(dealyBetweenPulses);
}


void onestepbackward(){
stepperWrite(1,0,0,0);
delay(dealyBetweenPulses);
stepperWrite(1,0,0,1);
delay(dealyBetweenPulses);
stepperWrite(0,0,0,1);
delay(dealyBetweenPulses);
stepperWrite(0,0,1,1);
delay(dealyBetweenPulses);
stepperWrite(0,0,1,0);
delay(dealyBetweenPulses);
stepperWrite(0,1,1,0);
delay(dealyBetweenPulses);
stepperWrite(0,1,0,0);
delay(dealyBetweenPulses);
stepperWrite(1,1,0,0);
delay(dealyBetweenPulses);
}


void openSafe(){
  while(i<NUMBER_OF_STEPS_PER_REV){
    onestepbackward();
    i++;
  }
  i=0;
}

void closeSafe(){
  while(i<NUMBER_OF_STEPS_PER_REV){
    onestep();
    i++;
  }
  i=0;
}


void enter_button() {
  SW_state = digitalRead(buttonEnterPin);
  if (SW_state == HIGH) {
    debouncerEnter = true;
  }
  if (SW_state == LOW && debouncerEnter) {
    if(opened){
        closeSafe();
        opened = false;
        digitalWrite(ledPinR, true);
        digitalWrite(ledPinG, false);
        return;
    }
    data[row][0] = north;
    data[row][1] = south;
    data[row][2] = east;
    data[row][3] = west;
    ++row;
    if (row == maxRow) {
          if (checkPassword()) {
            digitalWrite(ledPinR, false);
            digitalWrite(ledPinG, true);
            opened = true;
            openSafe();
            
          } else {
            digitalWrite(Beeper, HIGH);
            delay(500);
            digitalWrite(Beeper, LOW);
          }
          row = 0;
    }
    clear_coordinates();
    lcd_print();
    debouncerEnter = false;

  }
}

void clear_coordinates() {
  north = 0;
  south  = 0;
  east  = 0;
  west  = 0;
}



void clearData() {
  for (int i = 0; i < maxRow ; ++i) {
    for (int a = 0; a < 4 ; ++a) {
      data[i][a] = 0;
    }
  }
}

void correctInsert() {
  for (int i = 0; i < 2; ++i) {
    digitalWrite(Beeper, HIGH);
    digitalWrite(ledPinR, LOW);
    delay(50);
    digitalWrite(Beeper, LOW);
    digitalWrite(ledPinR, HIGH);
    delay(50);
  }

}


boolean checkPassword() {
  for (int i = 0; i < maxRow ; ++i) {
    for (int a = 0; a < 4 ; ++a) {
      if (data[i][a] != password[i][a]) {
        return false;
      }
    }

  }

  return true;
}

void joystick_direction_setter() {
  xPosition = analogRead(VRx);
  yPosition = analogRead(VRy);


  if (xPosition >= 1000) {
    if (west == 0 && east < 10) {
      ++east;
      lcd_print();
    }
    if (west != 0 && east == 0) {
      --west;
      lcd_print();
    }
  }
  if (xPosition <= 15) {
    if (east == 0 && west < 10) {
      ++west;
      lcd_print();
    }
    if (east != 0 && west == 0) {
      --east;
      lcd_print();
    }
  }

  if (yPosition >= 1000) {
    if (north == 0 && south < 10) {
      ++south;
      lcd_print();
    }
    if (north != 0 && south == 0) {
      --north;
      lcd_print();
    }

  }

  if (yPosition <= 15) {
    if (south == 0 && north < 10) {
      ++north;
      lcd_print();
    }
    if (south != 0 && north == 0) {
      --south;
      lcd_print();
    }
  }
}

void lcd_print() {
  lcd.clear();
  lcd.setCursor(0, 0);


  char buffer [18]; // a few bytes larger than your LCD line

  sprintf (buffer, "N:%02u ", north); // send data to the buffer
  lcd.print (buffer);
  sprintf (buffer, "S:%02u", south); // send data to the buffer
  lcd.print (buffer);

  sprintf (buffer, "    P:%01u", row + 1); // send data to the buffer
  lcd.print (buffer);




  lcd.setCursor(0, 1);
  sprintf (buffer, "E:%02u ", east); // send data to the buffer
  lcd.print (buffer);
  sprintf (buffer, "W:%02u", west); // send data to the buffer
  lcd.print (buffer);
}



void card_reader() {
  // Look for new cards
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    //Show UID on serial monitor
    String content = "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
    if (content.substring(1) == "D9 90 07 E5") {
      if (!lcd_acces_granted) {
        lcd.backlight();
        lcd.display();
        lcd_print();
        lcd_acces_granted = true;
        for (int i = 0; i < 2; ++i) {
          digitalWrite(Beeper, HIGH);
          delay(50);
          digitalWrite(Beeper, LOW);
          delay(50);
        }
      }
    } else {
      Serial.println(" Access denied");
      lcd.noBacklight();
      lcd.noDisplay();
      lcd_acces_granted = false;
      digitalWrite(Beeper, HIGH);
      delay(500);
      digitalWrite(Beeper, LOW);
    }
  }
}
