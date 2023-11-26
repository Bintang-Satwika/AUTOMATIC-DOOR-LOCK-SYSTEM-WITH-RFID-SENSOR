#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define RST_PIN 5
#define SS_PIN  53

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
