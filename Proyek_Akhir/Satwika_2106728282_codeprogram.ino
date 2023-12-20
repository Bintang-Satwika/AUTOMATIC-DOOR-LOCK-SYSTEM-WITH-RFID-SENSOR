#define BLYNK_TEMPLATE_ID "TMPL6PLZ5BuCy"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "0ESDh-z-4v56tUH0CmQf8MXhl2VFZxgu"
#include <MFRC522.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
#include "SD.h"
#include "SPI.h"
#include <BlynkSimpleEsp32.h>
#include <RTClib.h>
#include <ESP32Servo.h>

#define RST_PIN 13 // RFID RST
#define SS_PIN  33 // RFID SDA(SS)
#define Buzzer 4
#define SERVO_PIN 2
#define YellowLED 16
#define GreenLED 17
#define RedLED 15
#define CS_PIN 5

MFRC522 rfid(SS_PIN, RST_PIN); // RFID Sensor
LiquidCrystal_I2C lcd(0x27, 16, 2); //LCD
RTC_DS3231 rtc; // RTC DS3231
Servo servo; // servo
File myFile; // SD CARD

String waktu_sekarang="";
String MasterTag = "73 DB 1E 03"; // Enter your tag UID which we get from the first code.
String UIDCard = "";
String nama = "unknown";
bool logic_blynk = false;
bool logic_rfid = false;
int lock=0;
TaskHandle_t first_task = NULL;
TaskHandle_t secondary_task = NULL;

char ssid[] = "A3";
char pass[] = "zorabin75";

void first(void *param);
void secondary(void *param);

void setup() {
  Serial.begin(115200);
  
  Wire.begin();
  SPI.begin();
  rfid.PCD_Init();
  pinMode(Buzzer, OUTPUT);
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // RTC MODULE INISIASI
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  } 
    if (!SD.begin(CS_PIN)) {
    Serial.println("Initialization failed!");
    return;
  } else {
    Serial.println(F("Initialization done."));

    if (SD.exists("/LOG.txt") == 0) { // menggunakan "/" di awal penamaan 
      Serial.print(F("\r\nCreating 'LOG.txt' file..."));
      myFile = SD.open("/LOG.txt", FILE_WRITE); // menggunakan "/" di awal penamaan 
      if (myFile) {
        Serial.println(F("'LOG.txt' file created."));
        myFile.println("Date & Time, ID CARD, nama");
        myFile.close();
      } else 
        Serial.println(F("Error creating file or file already exists."));
      
    }
     myFile = SD.open("/LOG.txt", FILE_APPEND); // menggunakan "/" di awal penamaan dan FILE_APPEND
    if (!myFile) {
      Serial.println(F("Error opening LOG.txt"));
      }
  } // else -> Serial.println(F("Initialization done."));

   
  // PWM pin (Servo & Buzzer)
  pinMode(Buzzer, OUTPUT);
  servo.attach(SERVO_PIN);
  servo.write(0);
  
  // digital Pin( LED)
  pinMode(GreenLED, OUTPUT);
  pinMode(YellowLED, OUTPUT);
  pinMode(RedLED, OUTPUT);
  pinMode(Buzzer, OUTPUT);

  
  //LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print(" Access Control ");
  lcd.setCursor(0, 1);
  lcd.print("Scan Your Card");
  
  xTaskCreatePinnedToCore(first, "first_task", 10000, NULL, 1, &first_task, 0);
  xTaskCreatePinnedToCore(secondary, "secondary_task", 10000, NULL, 1, &secondary_task, 1);
  
}

void loop() {
    Blynk.run();
}

void first(void *param) {
  for (;;) {
  
  digitalWrite(YellowLED, HIGH);
  digitalWrite(RedLED, LOW);
  digitalWrite(GreenLED, LOW);
  if(logic_rfid==true){
   Serial.print("UID: ");
   Serial.println(String(UIDCard));
  }
  if ((UIDCard == MasterTag && logic_rfid==true) || logic_blynk == true) {
             
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("Permission");
        lcd.setCursor(0, 1);
        lcd.print(" Access Granted!");
        
        digitalWrite(GreenLED, HIGH);
        digitalWrite(YellowLED, LOW);
        digitalWrite(RedLED, LOW);
        
        for (int i = 0; i < 2; i++) {
          tone(Buzzer, 2000);
          delay(250);
          noTone(Buzzer);
          delay(250);
        }
        lcd.clear();  
    }
    else if((UIDCard != MasterTag &&  logic_rfid==true)&& logic_blynk == false ){
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Permission");
      lcd.print(" Access Denied!");
      digitalWrite(YellowLED, LOW);
      digitalWrite(GreenLED, LOW);
      digitalWrite(RedLED, HIGH);
      tone(Buzzer, 2000);
      for(int i = 0; i < 3;i++){
        digitalWrite(RedLED, HIGH);
        delay(250);
        digitalWrite(RedLED, LOW);
        delay(250);
        }
      noTone(Buzzer);
       lcd.clear();  
    }
    lcd.print(" Access Control ");
    lcd.setCursor(0, 1);
    lcd.print("Scan Your Card");
    

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void secondary(void *param) {
  for (;;) {
    logic_rfid= rfid_module();
    if (logic_rfid==true || logic_blynk == true){
        if(UIDCard == MasterTag ){
            if (lock==1){
                Serial.println("servo putasr 0 derajat");
                servo.write(0);
                delay(100);
                lock=0;
              }
              else if (lock==0){
                Serial.println("servo putar 90 derajat");
                servo.write(90);
                delay(100);
                lock=1;
              }
          }
          
        else if (logic_blynk == true){
           UIDCard  = MasterTag;
           nama     = "Bintang";
           logic_blynk= false;
          }
           data_logging(); // simpan di sd card
      }
      Blynk.virtualWrite(V1, UIDCard);
      Blynk.virtualWrite(V2, nama);
      Blynk.virtualWrite(V3, waktu_sekarang);
    
    vTaskDelay(100 / portTICK_PERIOD_MS); 
  }
}

void data_logging(){
   DateTime now = rtc.now();
   waktu_sekarang =now.year() + '/' +
                        String(now.month(), DEC) + '/' +
                        String(now.day(), DEC) + ' ' +
                        String(now.hour(), DEC) + ':' +
                        String(now.minute(), DEC) + ':' +
                        String(now.second(), DEC);
                       
    if (!myFile) {
      Serial.println(F("Error opening LOG.txt"));
      }
    else {
     myFile.print("\n" + String(waktu_sekarang) + "," + String(UIDCard) + "," + String(nama));
     myFile.flush();  
    }
    Serial.println(String(waktu_sekarang)+","+String(UIDCard)+","+String(nama));
  }

bool rfid_module(){
  if (!rfid.PICC_IsNewCardPresent()) {
    return false;
  }

  else if (!rfid.PICC_ReadCardSerial()) {
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
  if(UIDCard == MasterTag )nama = "Bintang";
  else nama = "Unknown";
  
  return true;

}

BLYNK_WRITE(V0) { // Switch widget on V4 updates logic_blynk
  logic_blynk = param.asInt();
}
