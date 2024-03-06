#include <WiFi.h>
#include "aaFlipra.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

String serverName = "http://worldtimeapi.org/api/timezone/Europe/Zurich";

#define zeile1 5

aaFlipra ww;

const uint8_t ziffer[][4] = {
  { 0x7C, 0x82, 0x82, 0x7C },
  { 0x00, 0x42, 0xFE, 0x02 },
  { 0x46, 0x8A, 0x92, 0x62 },
  { 0x44, 0x82, 0x92, 0x6C },
  { 0x30, 0x50, 0x90, 0xFE },
  { 0xE4, 0xA2, 0xA2, 0x9C },
  { 0x7C, 0xA2, 0xA2, 0x1C },
  { 0x86, 0x88, 0x90, 0xE0 },
  { 0x6C, 0x92, 0x92, 0x6C },
  { 0x70, 0x8A, 0x8A, 0x7C },
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
      anzeigeByte = anzeigeByte << 1;
      delay(2);
    }
  }
}

String getTimeFromServer() {
  String timeHHmm = "9999";

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName.c_str());
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, response);

      if (!error) {
        String utcDatetime = doc["datetime"];
        int utcOffsetSeconds = doc["utc_offset"];

        int utcOffsetMillis = utcOffsetSeconds * 1000;

        struct tm tm;
        strptime(utcDatetime.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
        time_t utcTime = mktime(&tm);

        time_t localTime = utcTime + utcOffsetMillis / 1000;

        struct tm *localTm = localtime(&localTime);
        char buf[5];
        strftime(buf, sizeof(buf), "%H%M", localTm);
        timeHHmm = buf;
      }
      else {
        return "9999";
      }
    }
    http.end();
  }

  return timeHHmm;
}

void setup() {
  ww.begin();
  ww.dotPowerOn();
  ww.setCoilFlipDuration(600);

  ww.resetAll(0);

  WiFi.begin(SSID, PASSWORD);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (i % 2 == 0) {
      ww.setDot(2, 2);
    } else {
      ww.resetDot(2, 2);
    }

    i++;
    delay(1000);
  }

  ww.resetAll(0);
  delay(10000);
}

void loop() {
  String currentTime;

  currentTime = getTimeFromServer();

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
  delay(30000);
}
