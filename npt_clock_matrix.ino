#include <WiFi.h>
#include "aaFlipra.h"
#include <HTTPClient.h>

String serverName = "http://worldtimeapi.org/api/timezone/Europe/Zurich.txt";

#define zeile1 5  // obere Position der Stunden-Ziffern

aaFlipra ww;


const uint8_t ziffer[][4] = {
  { 0x7C, 0x82, 0x82, 0x7C },  // 0
  { 0x00, 0x42, 0xFE, 0x02 },  // 1
  { 0x46, 0x8A, 0x92, 0x62 },  // 2
  { 0x44, 0x82, 0x92, 0x6C },  // 3
  { 0x30, 0x50, 0x90, 0xFE },  // 4
  { 0xE4, 0xA2, 0xA2, 0x9C },  // 5
  { 0x7C, 0xA2, 0xA2, 0x1C },  // 6
  { 0x86, 0x88, 0x90, 0xE0 },  // 7
  { 0x6C, 0x92, 0x92, 0x6C },  // 8
  { 0x70, 0x8A, 0x8A, 0x7C },  // 9
};


String lastTime = "0000";

void drawChar(int charNumber, int x0, int y0) {
  uint8_t anzeigeByte = 0;

  for (int xc = 0; xc < 4; xc++) {
    anzeigeByte = ziffer[charNumber][xc];
    for (int yc = 0; yc < 7; yc++) {
      if ((anzeigeByte & 128) == 128) {
        ww.setDot(x0 + xc, y0 + yc);
      } else {
        ww.resetDot(x0 + xc, y0 + yc);
      }
      anzeigeByte = anzeigeByte << 1;  //select next bit
      delay(2);                        // je nach gewünschter Flippgeschwindigkeit hier einen Wert von 0 bis 50? eingeben
    }
  }
}

String getTime() {
  String timeHHmm  = "9999";
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName.c_str());
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();
      // Parse response
      int utcDatetimeIndex = response.indexOf("utc_datetime:");
      if (utcDatetimeIndex != -1) {
        String utcDatetime = response.substring(utcDatetimeIndex + 25, utcDatetimeIndex + 30);
        timeHHmm = utcDatetime.substring(0, 2) + utcDatetime.substring(3, 5);
      }
    }
    http.end();
  }

  return timeHHmm;
}

void setup() {
  ww.begin();
  ww.dotPowerOn();  // schaltet die Flipspannung auf die Treiberbausteine
  ww.setCoilFlipDuration(600);

  ww.resetAll(0);  // setze alle Dots mit einer Verzögerungszeit von x Millisekunden zwischen jedem Dot

  delay(1000);

  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  delay(10000);
}

void loop() {
  String currentTime;

  currentTime = getTime();

  if (currentTime == lastTime) {
    return;
  }

  int hour1 = currentTime.substring(0, 1).toInt();
  int hour2 = currentTime.substring(1, 2).toInt();
  int minute1 = currentTime.substring(2, 3).toInt();
  int minute2 = currentTime.substring(3, 4).toInt();

  ww.resetAll(0);
  //1 Stunde
  drawChar(hour1, 4, zeile1);
  //2 Stunde
  drawChar(hour2, 9, zeile1);
  
  ww.setDot(14, 6);
  ww.setDot(14, 9);

  //1 Minute
  drawChar(minute1, 16, zeile1);
  //2 Minute
  drawChar(minute2, 21, zeile1);

  lastTime = currentTime;
  delay(30000);  // Delay for 1 minute before getting the time again
}
