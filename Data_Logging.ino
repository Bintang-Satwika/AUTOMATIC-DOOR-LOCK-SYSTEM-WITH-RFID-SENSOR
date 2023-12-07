#include <MFRC522.h> // for RFID
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Wire.h>   // Include library Wire
#include <DS3231.h> // Include library DS3231
#include <SPI.h>    // Include library SPI
#include <SD.h>     // Include library SD

#define RST_PIN 5 // arduino mega
#define SS_PIN  53 // arduino mega
#define CS_PIN 4 
#define SERVO_PIN A0
#define YellowLED  102
#define GreenLED 101
#define RedLED 100
#define Buzzer 2

DS3231  rtc(SDA, SCL); // rtc module for time
File    myFile; // SD CARD

MFRC522 rfid(SS_PIN, RST_PIN);// RFID Sensor
String MasterTag = "73 6C C8 12"; // Enter you tag UID which we get it from first code. 
String UIDCard = "";

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD
Servo servo; // SERVO

int lock=1;
String state="false";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);

  SPI.begin();
  rfid.PCD_Init();

  servo.attach(SERVO_PIN);
  servo.write(0);

  pinMode(GreenLED, OUTPUT);
  pinMode(YellowLED, OUTPUT);
  pinMode(RedLED, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  digitalWrite(YellowLED, HIGH);
    
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print(" Access Control ");
  lcd.setCursor(0, 1);
  lcd.print("Scan Your Card");
  
    // Menuliskan ke serial monitor
  Serial.println(F(" "));
  delay(500);
  
  // RTC MODULE INISIASI
  rtc.begin();
  //rtc.setDate(06, 12, 2023);   // Sesuaikan tanggal dengan tanggal praktikum Anda (dd, mm, yyyy)
  //rtc.setTime(11, 02, 00);     // Sesuaikan jam dengan waktu Anda (hh, mm, ss)
  //rtc.setDOW(WEDNESDAY);     // Sesuaikan hari dengan hari Anda praktikum (SUNDAY, MONDAY, etc.)

  
  //  SD CARD INISIASI
  Serial.print(F("Initializing SD card..."));
  if(!SD.begin(CS_PIN)) {
    Serial.println("initialization failed!");
    return;
  }
  else {
    Serial.println(F("Initialization done."));
    
    if( SD.exists("LOG.txt") == 0 ) {
      Serial.print(F("\r\nCreating 'LOG.txt' file..."));
      myFile = SD.open("LOG.txt", FILE_WRITE);
      if(myFile) {
        Serial.println(F("'LOG.txt' file created."));        
        myFile.println("Date & Time, ID CARD, condition");
        myFile.close();
      }
      else
        Serial.println(F("Error creating file or file already exist."));
    }
      
  }
}

void loop() {
  Serial.println("Mulai:");
  // put your main code here, to run repeatedly:
  digitalWrite(YellowLED, HIGH);
  digitalWrite(RedLED, LOW);
  digitalWrite(GreenLED, LOW);
  noTone(Buzzer);
  //Wait until new tag is available
  while(getUID()){
    Serial.print("UID: ");
    Serial.println(UIDCard);
    loop_logging(); // DATA LOGGING
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Permission");
    lcd.setCursor(0, 1);
    if (UIDCard == MasterTag && lock ==1){
      lcd.print(" Access Granted!");
      digitalWrite(GreenLED, HIGH);
      digitalWrite(YellowLED, LOW);
      digitalWrite(RedLED, LOW);
      servo.write(100);
      delay(50);
      lock=0;
      state = "true";
      for (int i = 0; i < 2; i++) {
        tone(Buzzer, 2000);
        delay(250);
        noTone(Buzzer);
        delay(250);
      }
    }
    else if(UIDCard == MasterTag && lock ==0){
      lcd.print(" Access Granted!");
      digitalWrite(GreenLED, HIGH);
      digitalWrite(YellowLED, LOW);
      digitalWrite(RedLED, LOW);
      servo.write(0);
      delay(50);
      lock=1;
      state = "true";
      for (int i = 0; i < 2; i++) {
        tone(Buzzer, 2000);
        delay(250);
        noTone(Buzzer);
        delay(250);
        }  
      }
    else{
      lcd.print(" Access Denied!");
      digitalWrite(YellowLED, LOW);
      digitalWrite(GreenLED, LOW);
      state="false";
      tone(Buzzer, 2000);
      for(int i = 0; i < 3;i++){
        digitalWrite(RedLED, HIGH);
        delay(250);
        digitalWrite(RedLED, LOW);
        delay(250);
        }
      noTone(Buzzer);
    }
    delay(1000);
    lcd.clear();
    lcd.print(" Access Control ");
    lcd.setCursor(0, 1);
    lcd.print("Scan Your Card");
  } // IF GET UID EALSE
  
} // VOID LOOP END

void loop_logging(){
  char buffer1[12], tempStr[6], buffer2[20];
  myFile = SD.open("LOG.txt", FILE_WRITE);
  Serial.println("mASUK LOOP");
  if (myFile) {
    Serial.println("MAU PRINT LOGGING 222222");
    sprintf(buffer1, "Date: %s, Time: %s, Day of the Week: %s\n", rtc.getDateStr(), rtc.getTimeStr(), rtc.getDOWStr());
    Serial.print(buffer1);
    Serial.print("ID CARD:" +String(UIDCard));
    // SAVE TO SD CARD
    myFile.print(rtc.getDateStr());
    myFile.print(" ");
    myFile.print(rtc.getTimeStr());
    myFile.print(",");
    myFile.print(String(UIDCard));
    myFile.print(",");
    myFile.print(String(state));
    myFile.print("\n");
    myFile.close();
  }
  else Serial.println(F("Error opening LOG.txt")); 
  
  }

bool getUID(){
  if (! rfid.PICC_IsNewCardPresent()) {
    return false;
  }

  if (! rfid.PICC_ReadCardSerial()) {
    Serial.println("Not able to read the card");
    return false;
  }
  UIDCard = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    UIDCard.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    UIDCard.concat(String(rfid.uid.uidByte[i], HEX));
  }
  UIDCard.toUpperCase();
  UIDCard = UIDCard.substring(1);

  rfid.PICC_HaltA();
  return true;
}
