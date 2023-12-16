#define BLYNK_TEMPLATE_ID "TMPL6PLZ5BuCy"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "0ESDh-z-4v56tUH0CmQf8MXhl2VFZxgu"
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
#include <SD.h>
#include <BlynkSimpleEsp32.h>
#include <RTClib.h>

#define RST_PIN 13 // RFID RST
#define SS_PIN  33 // RFID SDA(SS)
#define Buzzer 4

MFRC522 rfid(SS_PIN, RST_PIN); // RFID Sensor
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

String waktu_sekarang="";
String MasterTag = "73 6C C8 12"; // Enter your tag UID which we get from the first code.
String UIDCard = "";
String nama = "unknown";
bool logic_blynk = false;
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
  pinMode(Buzzer, OUTPUT);
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
    if (rfid_module()) {
      Serial.println("berhasil");
      Serial.println(String(UIDCard));
      rtc_module();
      Serial.println(waktu_sekarang);
    if (UIDCard == MasterTag) {
        nama = "Bintang";
      tone(Buzzer, 2000);
      delay(250);
      noTone(Buzzer);
      }
     else nama= "unkonwn";
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
void secondary(void *param) {
  for (;;) {
    if(logic_blynk){
     nama    = "Bintang";
     UIDCard  = "Online";
     rtc_module();
      }
      Blynk.virtualWrite(V1, UIDCard);
      Blynk.virtualWrite(V2, nama);
      Blynk.virtualWrite(V3, waktu_sekarang);
    
    vTaskDelay(100 / portTICK_PERIOD_MS); 
  }
}

void rtc_module(){
   DateTime now = rtc.now();
   waktu_sekarang =now.year() + '/' +
                        String(now.month(), DEC) + '/' +
                        String(now.day(), DEC) + ' ' +
                        String(now.hour(), DEC) + ':' +
                        String(now.minute(), DEC) + ':' +
                        String(now.second(), DEC);
  }

bool rfid_module(){
  if (!rfid.PICC_IsNewCardPresent()) {
    return false;
  }

  if (!rfid.PICC_ReadCardSerial()) {
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

BLYNK_WRITE(V0) { // Switch widget on V4 updates switchState
  logic_blynk = param.asInt();
}
