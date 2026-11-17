#include <WiFiUdp.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiScan.h>
#include <WiFiMulti.h>
#include <WiFi.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiGeneric.h>
#include <Adafruit_ST7796S_kbv.h>

/***************************************************
  This is our library for the Adafruit HX8357D Breakout
  ----> http://www.adafruit.com/products/2050

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <time.h>
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ST7796S_kbv.h"

#include <Bonezegei_DS1307.h>

Bonezegei_DS1307 rtc(0x68);

// These are 'flexible' lines that can be changed
#define TFT_CS 26
#define TFT_DC 14
#define TFT_RST 27 // RST can be set to -1 if you tie it to Arduino's reset

const char *ssid = "ARRIS-959D";
const char *password = "4Ym6jHhd4y7y";
const char *ntpServer = "pool.ntp.org";

const long gmtOffset_sec = -18000;
const char *tzInfo = "EST5EDT,M3.2.0,M11.1.0";

bool timeReady = false;
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ST7796S_kbv tft = Adafruit_ST7796S_kbv(TFT_CS, TFT_DC, TFT_RST);
// SoftSPI - note that on some processors this might be *faster* than hardware SPI!
//Adafruit_ST7796S_kbv tft = Adafruit_ST7796S_kbv(TFT_CS, TFT_DC, MOSI, SCK, TFT_RST, MISO);

uint16_t convertHEXto565(uint32_t hexColor) {
  uint r = (hexColor >> 16) & 0xFF;
  uint g = (hexColor >> 8) & 0xFF;
  uint b = hexColor >> 8 & 0xFF;

  return static_cast<uint16_t>(((r & 0xF8) << 8) |
                               ((g & 0xFC) << 3) |
                               (b >> 3));

}

void initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname("meow, meow meow");
  WiFi.begin(ssid, password);
  // Auto reconnect is set true as default
  // To set auto connect off, use the following function
  //    WiFi.setAutoReconnect(false);

  // Will try for about 10 seconds (20x 500ms)
  int tryDelay = 500;
  int numberOfTries = 20;

  // Wait for the WiFi event
  while (true) {

    switch (WiFi.status()) {
      case WL_NO_SSID_AVAIL: Serial.println("[WiFi] SSID not found"); break;
      case WL_CONNECT_FAILED:
        Serial.print("[WiFi] Failed - WiFi not connected! Reason: ");
        return;
        break;
      case WL_CONNECTION_LOST: Serial.println("[WiFi] Connection was lost"); break;
      case WL_SCAN_COMPLETED:  Serial.println("[WiFi] Scan is completed"); break;
      case WL_DISCONNECTED:    Serial.println("[WiFi] WiFi is disconnected"); break;
      case WL_CONNECTED:
        Serial.println("[WiFi] WiFi is connected!");
        Serial.print("[WiFi] IP address: ");
        Serial.println(WiFi.localIP());
        return;
        break;
      default:
        Serial.print("[WiFi] WiFi Status: ");
        Serial.println(WiFi.status());
        break;
    }
    delay(tryDelay);

    if (numberOfTries <= 0) {
      Serial.print("[WiFi] Failed to connect to WiFi!");
      // Use disconnect function to force stop trying to connect
      WiFi.disconnect();
      return;
    } else {
      numberOfTries--;
    }
  }
}

void initTime() {
  // try 3 times
  Serial.println("[Time] Initializing NTP");
  int tryCount = 3;
  while (tryCount != 0) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[Time] Not connected to internet! Retrying...");
      delay(500);
      tryCount--;
      continue;
    } else {
      Serial.println("[Time] NTP is ready");
      timeReady = true;
      setTime();
      break;
    }
  }
}

void setTime() {
  if (!timeReady) return;
  /* struct tm timeinfo;
  getLocalTime(&timeinfo); */
  // TODO: Ask nist time servers;
  int hour = timeinfo.tm_hour;
  int minute = timeinfo.tm_min;
  int second = timeinfo.tm_sec;

  int month = timeinfo.tm_mon + 1;
  int day = timeinfo.tm_mday;
  int year = timeinfo.tm_year;
  Serial.print(hour);
  Serial.print(minute);
  Serial.print(second);
  Serial.print(month);
  Serial.print(day);
  Serial.print(year);

  char time[16], date[16];
  sprintf(time, "%02d:%02d:%02d", hour, minute, second);
  sprintf(date, "%02d/%02d/%02d", month, day, year);
  rtc.setTime(time);
  rtc.setDate(date);
  rtc.setDay(timeinfo.tm_wday);
}

void setup() {
//  pinMode(7,OUTPUT);/
//  digitalWrite(7,HIGH);/
  Serial.begin(9600);
  rtc.begin();
  tft.begin();

  initWifi();
  initTime();
}


void loop(void) {
  tft.fillScreen(ST7796S_BLACK);
  tft.setRotation(3);
  tft.fillRoundRect(0, 0, 480, 18, 0, convertHEXto565(0xD9AE94));
  if (rtc.getTime())
  {
    // Serial.printf only does not work on atmega
    // Serial.printf("Time %02d:%02d:%02d ", rtc.getHour(), rtc.getMinute(), rtc.getSeconds());
    char time_data[32];
    sprintf(time_data, "%02d:%02d:%02d ", rtc.getHour(), rtc.getMinute(), rtc.getSeconds());
    char date_data[32];
    sprintf(date_data, "%02d/%02d/%02d", rtc.getMonth(), rtc.getDate(), rtc.getYear());
    tft.setCursor(2, 2);
    tft.setTextColor(ST7796S_BLACK, convertHEXto565(0xD9AE94));
    tft.setTextSize(2);
    tft.print(time_data);
    tft.setCursor(384, 2);
    tft.print(date_data);
  }
    delay(10000);
  } /*

  //  digitalWrite(7,HIGH);
    tft.fillScreen(ST7796S_BLACK);
    tft.setRotation(0);
    tft.setTextSize(2);
    tft.setCursor(0, 0);
    tft.println("11/08/26      | 11:18");
    tft.print("Future Home Automation Hub");
    tft.println("TODO: display site statuses");
    tft.println("http://whiskers.one - status: ???");
    tft.println("http://whiskers.wtf - status: ???");
    tft.println("http://catz.wtf     - status: ???");
    tft.println("http://whiskers.one - status: ???");
    tft.println("TODO: display light controls");
    tft.println("TODO: create rgb status indicator");
    tft.println("TODO: CREATE UI");
    tft.println("TODO: Integrate light controlling");
    tft.println("Weather for Lansing, MI: ??°c");
    tft.println("it's probably cold");
    delay(30000);

    tft.fillScreen(ST7796S_BLACK);
    tft.setRotation(2);
    tft.setTextSize(2);
    tft.setCursor(0, 0);
    tft.println("11/08/26      | 11:19");
    tft.print("Future Home Automation Hub");
    tft.println("TODO: display site statuses");
    tft.println("http://whiskers.one - status: ???");
    tft.println("http://whiskers.wtf - status: ???");
    tft.println("http://catz.wtf     - status: ???");
    tft.println("http://whiskers.one - status: ???");
    tft.println("TODO: display light controls");
    tft.println("TODO: create rgb status indicator");
    tft.println("TODO: CREATE UI");
    tft.println("TODO: Integrate light controlling");
    tft.println("Weather for Lansing, MI: ??°c");
    tft.println("it's probably cold");
    delay(30000);


  }

  //  */
