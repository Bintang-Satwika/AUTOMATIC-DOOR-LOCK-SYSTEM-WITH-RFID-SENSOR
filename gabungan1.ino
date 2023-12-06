#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Wire.h>   // Include library Wire
#include <DS3231.h> // Include library DS3231
#include <SPI.h>    // Include library SPI
#include <SD.h>     // Include library SD

DS3231  rtc(SDA, SCL);
File    myFile;

#define RST_PIN 5 // arduino mega
#define SS_PIN  53 // arduino mega


MFRC522 mfrc522(SS_PIN, RST_PIN);
String MasterTag = "73 6C C8 12"; // Enter you tag UID which we get it from first code. 

String UIDCard = "";

int lock=1;

LiquidCrystal_I2C lcd(0x27, 16, 2);

Servo servo;

#define YellowLED  2
#define GreenLED 3
#define RedLED 4

#define Buzzer 7
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  SPI.begin();
  mfrc522.PCD_Init();

  lcd.init();
  lcd.backlight();
  lcd.clear();

  servo.attach(A0);
  servo.write(0);

  pinMode(GreenLED, OUTPUT);
  pinMode(YellowLED, OUTPUT);
  pinMode(RedLED, OUTPUT);

  pinMode(Buzzer, OUTPUT);

  digitalWrite(YellowLED, HIGH);

  lcd.clear();
  lcd.print(" Access Control ");
  lcd.setCursor(0, 1);
  lcd.print("Scan Your Card");
  setup_logging();
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(YellowLED, HIGH);
  digitalWrite(RedLED, LOW);
  digitalWrite(GreenLED, LOW);
  noTone(Buzzer);
  //Wait until new tag is available
  while (getUID()){
    Serial.print("UID: ");
    Serial.println(UIDCard);
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
    loop_logging();
    delay(1000);
    
  }
  
}
void setup_logging(){
  
    while (!Serial) {
    ;
  }
  
  // Menuliskan ke serial monitor
  Serial.println(F("Program 3: Data Logging"));
  Serial.println(F("Praktikum Sistem Benam (Embedded System)"));
  Serial.println(F(" "));
  delay(500);
  
  rtc.begin();
  
  // Setting tanggal dan waktu pertama program
  rtc.setDate(06, 12, 2023);   // Sesuaikan tanggal dengan tanggal praktikum Anda (dd, mm, yyyy)
  rtc.setTime(11, 02, 00);     // Sesuaikan jam dengan waktu Anda (hh, mm, ss)
  rtc.setDOW(WEDNESDAY);     // Sesuaikan hari dengan hari Anda praktikum (SUNDAY, MONDAY, etc.)

  Serial.print(F("Initializing SD card..."));
 
  if (!SD.begin(4)) {
    Serial.println(F("Initialization failed!"));
    while (1);
  }
  else {
      Serial.println(F("Initialization done."));
      
      if( SD.exists("LOG.txt") == 0 ) {
        Serial.print(F("\r\nCreating 'LOG.txt' file..."));
        myFile = SD.open("LOG.txt", FILE_WRITE);
        if(myFile) {
          Serial.println(F("'LOG.txt' file created."));        
          myFile.println("Date & Time, Temperature");
          myFile.close();
        }
        else
          Serial.println(F("Error creating file or file already exist.")); 
       }
        
    }
  }

void loop_logging(){
  char buffer1[12], tempStr[6];
  myFile = SD.open("LOG.txt", FILE_WRITE);
  if (myFile) {
    sprintf(buffer1, "Date: %s, Time: %s, Day of the Week: %s ", rtc.getDateStr(), rtc.getTimeStr(), rtc.getDOWStr());
    Serial.println(buffer1);
    // Serial.print(rtc.getTemp());
    //Serial.print(char(176));
   // Serial.println("C");

    myFile.print(rtc.getDateStr());
    myFile.print(" ");

    myFile.print(rtc.getTimeStr());
    myFile.close();
  }
  else {
    Serial.println(F("Error opening LOG.txt")); 
  }
  
  }

boolean getUID(){
  if (! mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  if (! mfrc522.PICC_ReadCardSerial()) {
    Serial.println("Not able to read the card");
    return false;
  }
  UIDCard = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    UIDCard.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    UIDCard.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  UIDCard.toUpperCase();
  UIDCard = UIDCard.substring(1);

  mfrc522.PICC_HaltA();
  return true;
}
