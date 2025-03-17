#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <NanoOLED.h>
#include <RTClib.h>

#define RE 8
#define DE 7

NanoOLED NanoOled(SH1106);
RTC_DS3231 rtc;

File dataFile;
String currentDatetime;

const byte nitro[] = { 0x01, 0x03, 0x00, 0x04, 0x00, 0x01, 0xC5, 0xCB };
const byte phos[]  = { 0x01,0x03, 0x00, 0x05, 0x00, 0x01, 0x94, 0x0B };
const byte pota[] = { 0x01,0x03, 0x00, 0x06, 0x00, 0x01, 0x64, 0x0B };
const byte moist[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A };
const byte temp[] = { 0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0xD5, 0xCA };
const byte ph[] = { 0x01, 0x03, 0x00, 0x03, 0x00, 0x01, 0x74, 0x0A };

const int chipSelect = 10;

byte values[7];

SoftwareSerial mod1(2, 3);

void setup() {
  Serial.begin(9600);

  while (!Serial) {
  }

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  mod1.begin(4800);
  NanoOled.init();

  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);

  if (!SD.begin(chipSelect)) {
      NanoOled.print("SD card initialization failed!");
      Serial.print("SD card initialization failed!");
      return;
  }
  DateTime now = rtc.now();
  currentDatetime = String(now.month()) + String(now.day()) + String(now.hour()) + String(now.minute());
  
  Serial.print(currentDatetime);
  dataFile = SD.open(currentDatetime + ".csv", FILE_WRITE);
  dataFile.println("Date, Time, Nitrogen, Phosphorus, Potassium, Moisture, Temperature, PH");
  dataFile.close();
}

void loop() {

  byte val1,val2,val3, val4, val5, val6, val7, val8, val9;
  val1 = nitrogen();
  delay(250);
  val2 = phosphorous();
  delay(250);
  val3 = potassium();
  delay(250);
  val4 = moisture();
  delay(250);
  val5 = temperature();
  delay(250);
  val6 = phValue();
  delay(250);

  NanoOled.clearDisplay();
  NanoOled.setCursor(0, 0);
  NanoOled.print(F("Nitrogen: "));
  NanoOled.print(val1);
  NanoOled.print(F(" mg/kg"));
  NanoOled.setCursor(1, 0);
  NanoOled.print(F("Phosphorus: "));
  NanoOled.print(val2);
  NanoOled.print(F(" mg/kg"));
  NanoOled.setCursor(2, 0);
  NanoOled.print(F("Potassium: "));
  NanoOled.print(val3);
  NanoOled.print(F(" mg/kg"));
  NanoOled.setCursor(3, 0);
  NanoOled.print(F("Moisture: "));
  NanoOled.print(val4/10.0);
  NanoOled.print(F(" %"));
  NanoOled.setCursor(4, 0);
  NanoOled.print(F("Temp: "));
  NanoOled.print(val5/10.0 + 24);
  NanoOled.print(F(" deg.C"));
  NanoOled.setCursor(5, 0);
  NanoOled.print(F("PH: "));
  NanoOled.print(val6/10.0);

  val7 = val4/10.0;
  val8 = val5/10.0 + 24;
  val9 = val6/10.0;


  DateTime present = rtc.now();
  String date = String(present.year()) + "-" + String(present.month()) + "-" + String(present.day());
  String time = String(present.hour()) +  ":" +  String(present.minute()) + ":" + String(present.second());

  dataFile = SD.open(currentDatetime + ".csv", FILE_WRITE);
  dataFile.println(date + "," + time + "," + val1 + "," + val2 + "," + val3 + "," + val7 + "," + val8 + "," + val9);
  dataFile.close();

  delay(3000);
}

byte nitrogen() {
  return readSensor(nitro, "Nitrogen");
}

byte phosphorous() {
  return readSensor(phos, "Phosphorous");
}

byte potassium() {
  return readSensor(pota, "Potassium");
}

byte moisture() {
  return readSensor(moist, "Moisture");
}

byte temperature() {
  return readSensor(temp, "Temperature");
}

byte phValue() {
  return readSensor(ph, "pH");
}

byte readSensor(const byte request[], const char* sensorName) {
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  if (mod1.write(request, 8) == 8) {
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    delay(100); 
    for (byte i = 0; i < 7; i++) {
      values[i] = mod1.read();
    }

  }
  return values[4];
}

