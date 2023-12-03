#include <Wire.h>   // Include library Wire
#include <DS3231.h> // Include library DS3231
#include <SPI.h>    // Include library SPI
#include <SD.h>     // Include library SD
 
DS3231  rtc(SDA, SCL);
File    myFile;

void setup() {
  Serial.begin(9600);
  
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
  rtc.setDate(22, 11, 2022);   // Sesuaikan tanggal dengan tanggal praktikum Anda (dd, mm, yyyy)
  rtc.setTime(11, 02, 00);     // Sesuaikan jam dengan waktu Anda (hh, mm, ss)
  rtc.setDOW(TUESDAY);     // Sesuaikan hari dengan hari Anda praktikum (SUNDAY, MONDAY, etc.)

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

void loop() {
  char buffer1[12], tempStr[6];
  myFile = SD.open("LOG.txt", FILE_WRITE);
  if (myFile) {
    sprintf(buffer1, "Date: %s, Time: %s, Day of the Week: %s, Temperature: ", rtc.getDateStr(), rtc.getTimeStr(), rtc.getDOWStr());
    Serial.print(buffer1);
    Serial.print(rtc.getTemp());
    Serial.print(char(176));
    Serial.println("C");

    myFile.print(rtc.getDateStr());
    myFile.print(" ");

    myFile.print(rtc.getTimeStr());
    myFile.print(",");
     
    myFile.println(rtc.getTemp(), 2);
    myFile.close();
  }
  else {
    Serial.println(F("Error opening LOG.txt")); 
  }
  delay(10000);
}
