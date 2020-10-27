#include <Arduino.h>

// ESP8266 Libraries
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// JSON Library
#include <ArduinoJson.h>

// SSD1306 Screen Libraries
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_ST7735.h> // Adafruit ST7735 and ST7789

// You need to add the followind data to inside /include/secrets.h
// const char *ssid = "YOUR_SSID";
// const char *password = "YOUR_PASSWORD";
// const char *url = "YOUR_OPENHARDWAREMONITOR_SERVER_URL";
// const char *hostname = "Hardware Monitor";
#include "secrets.h"

// Update over Wi-Fi Libraries
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Webserver Libraries
#include <ESP8266WebServer.h>

// Screen SSD1306
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);

// Screen ST7735
#define TFT_RST -1 //for TFT I2C Connector Shield V1.0.0 and TFT 1.4 Shield V1.0.0
#define TFT_CS 2   //for TFT I2C Connector Shield V1.0.0 and TFT 1.4 Shield V1.0.0
#define TFT_DC 0   //for TFT I2C Connector Shield V1.0.0 and TFT 1.4 Shield V1.0.0

// #define TFT_RST -1   //for TFT I2C Connector Shield (V1.1.0 or later)
// #define TFT_CS D0    //for TFT I2C Connector Shield (V1.1.0 or later)
// #define TFT_DC D8    //for TFT I2C Connector Shield (V1.1.0 or later)

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void startSSD1306()
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
}

void bootLogoSSD1306()
{
  // Display Network Status
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("Hardware Monitor");
  display.setCursor(0, 12);
  display.println("By Juanillo62gm");
  display.setCursor(0, 56);
  display.println("Connecting to Wi-Fi");

  // Actually display all of the above
  display.display();
}

void startST7735()
{
  tft.initR(INITR_144GREENTAB);
  tft.setTextWrap(false); // Allow text to run off right edge
  tft.fillScreen(ST7735_BLACK);
}

/*
void bootLogoST7735()
{
  tft.setTextColor(ST7735_BLUE, ST7735_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.println("Hardware Monitor");
  tft.setCursor(0, 12);
  tft.println("By Juanillo62gm");
  tft.setCursor(0, 56);
  tft.println("Connecting to Wi-Fi");
}
*/

void setup()
{
  Serial.begin(115200);
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  startSSD1306();

  startST7735();

  while (WiFi.status() != WL_CONNECTED)
  {
    bootLogoSSD1306();

    // bootLogoST7735();
  }

  // Update over Wi-Fi
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
    {
      Serial.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      Serial.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      Serial.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      Serial.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void hardwareMonitorSSD1306()
{
  WiFiClient client;
  HTTPClient http;

  // Send request
  http.useHTTP10(true);
  http.begin(client, url);
  http.GET();

  // Parse response
  const size_t capacity = 89 * JSON_ARRAY_SIZE(0) + 11 * JSON_ARRAY_SIZE(1) + 6 * JSON_ARRAY_SIZE(2) + 4 * JSON_ARRAY_SIZE(3) + 3 * JSON_ARRAY_SIZE(4) + 2 * JSON_ARRAY_SIZE(5) + JSON_ARRAY_SIZE(6) + JSON_ARRAY_SIZE(7) + 2 * JSON_ARRAY_SIZE(8) + 4 * JSON_ARRAY_SIZE(9) + 123 * JSON_OBJECT_SIZE(7) + 12530;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, http.getStream(), DeserializationOption::NestingLimit(12));

  String cpuName = doc["Children"][0]["Children"][1]["Text"];
  String cpuTempPackage = doc["Children"][0]["Children"][1]["Children"][1]["Children"][8]["Value"];
  String cpuLoad = doc["Children"][0]["Children"][1]["Children"][2]["Children"][0]["Value"];
  String gpuName = doc["Children"][0]["Children"][3]["Text"];
  String gpuHotSpot = doc["Children"][0]["Children"][3]["Children"][2]["Children"][5]["Value"];
  String gpuLoad = doc["Children"][0]["Children"][3]["Children"][3]["Children"][0]["Value"];
  String usedRAM = doc["Children"][0]["Children"][2]["Children"][1]["Children"][0]["Value"];
  String freeRAM = doc["Children"][0]["Children"][2]["Children"][1]["Children"][1]["Value"];

  String degree = degree.substring(degree.length()) + (char)247 + "C";
  String percentage = percentage.substring(percentage.length()) + (char)37;

  // Console Log
  Serial.print("\n\n\n\nCPU: ");
  Serial.println(cpuName);
  Serial.print("CPU Package: ");
  Serial.println(cpuTempPackage);
  Serial.print("CPU Load: ");
  Serial.println(cpuLoad);
  Serial.print("GPU Name: ");
  Serial.println(gpuName);
  Serial.print("GPU Hot Spot: ");
  Serial.println(gpuHotSpot);
  Serial.print("GPU Load: ");
  Serial.println(gpuLoad);
  Serial.print("Used RAM: ");
  Serial.println(usedRAM);
  Serial.print("Free RAM: ");
  Serial.println(freeRAM);

  // Text on Display 1
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // CPU
  // CPU - Name
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(cpuName);

  // CPU - Temperature and Load
  display.setTextSize(2);
  display.setCursor(1, 10);
  display.print("CPU");
  display.setCursor(40, 10);
  display.print(cpuTempPackage.substring(0, cpuTempPackage.length() - 6));
  display.setTextSize(1);
  display.print(degree);

  // CPU - Load
  display.setTextSize(2);
  display.setCursor(86, 10);
  display.print(cpuLoad.substring(0, cpuLoad.length() - 4));
  display.setTextSize(1);
  display.print(percentage);

  // GPU
  // GPU - Name
  display.setTextSize(1);
  display.setCursor(0, 28);
  display.println(gpuName);

  // GPU - Temperature
  display.setTextSize(2);
  display.setCursor(1, 38);
  display.print("GPU");
  display.setCursor(40, 38);
  display.print(gpuHotSpot.substring(0, gpuHotSpot.length() - 6));
  display.setTextSize(1);
  display.print(degree);

  // GPU - Load
  display.setTextSize(2);
  display.setCursor(86, 38);
  display.print(gpuLoad.substring(0, gpuLoad.length() - 4));
  display.setTextSize(1);
  display.print(percentage);

  // RAM
  display.setTextSize(1);
  display.setCursor(0, 56);
  display.println("Free RAM " + freeRAM);

  display.display();

  // Disconnect
  http.end();
}

void hardwareMonitorST7735()
{
  WiFiClient client;
  HTTPClient http;

  // Send request
  http.useHTTP10(true);
  http.begin(client, url);
  http.GET();

  // Parse response
  const size_t capacity = 89 * JSON_ARRAY_SIZE(0) + 11 * JSON_ARRAY_SIZE(1) + 6 * JSON_ARRAY_SIZE(2) + 4 * JSON_ARRAY_SIZE(3) + 3 * JSON_ARRAY_SIZE(4) + 2 * JSON_ARRAY_SIZE(5) + JSON_ARRAY_SIZE(6) + JSON_ARRAY_SIZE(7) + 2 * JSON_ARRAY_SIZE(8) + 4 * JSON_ARRAY_SIZE(9) + 123 * JSON_OBJECT_SIZE(7) + 12530;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, http.getStream(), DeserializationOption::NestingLimit(12));

  String cpuName = doc["Children"][0]["Children"][1]["Text"];
  String cpuTempPackage = doc["Children"][0]["Children"][1]["Children"][1]["Children"][8]["Value"];
  String cpuLoad = doc["Children"][0]["Children"][1]["Children"][2]["Children"][0]["Value"];
  String gpuName = doc["Children"][0]["Children"][3]["Text"];
  String gpuHotSpot = doc["Children"][0]["Children"][3]["Children"][2]["Children"][5]["Value"];
  String gpuLoad = doc["Children"][0]["Children"][3]["Children"][3]["Children"][0]["Value"];
  String usedRAM = doc["Children"][0]["Children"][2]["Children"][1]["Children"][0]["Value"];
  String freeRAM = doc["Children"][0]["Children"][2]["Children"][1]["Children"][1]["Value"];

  String degree = degree.substring(degree.length()) + (char)247 + "C";
  String percentage = percentage.substring(percentage.length()) + (char)37;

  // Console Log
  Serial.print("\n\n\n\nCPU: ");
  Serial.println(cpuName);
  Serial.print("CPU Package: ");
  Serial.println(cpuTempPackage);
  Serial.print("CPU Load: ");
  Serial.println(cpuLoad);
  Serial.print("GPU Name: ");
  Serial.println(gpuName);
  Serial.print("GPU Hot Spot: ");
  Serial.println(gpuHotSpot);
  Serial.print("GPU Load: ");
  Serial.println(gpuLoad);
  Serial.print("Used RAM: ");
  Serial.println(usedRAM);
  Serial.print("Free RAM: ");
  Serial.println(freeRAM);

  // Add screen content
  tft.setTextColor(ST7735_BLUE, ST7735_BLACK);

  // CPU
  // CPU - Name
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.println(cpuName);

  // CPU - Temperature and Load
  tft.setTextSize(2);
  tft.setCursor(1, 14);
  tft.print("CPU");
  tft.setCursor(40, 14);
  tft.print(cpuTempPackage.substring(0, cpuTempPackage.length() - 6));
  tft.setTextSize(1);
  tft.print(degree);

  // CPU - Load
  tft.setTextSize(2);
  tft.setCursor(86, 14);
  tft.print(cpuLoad.substring(0, cpuLoad.length() - 4));
  tft.setTextSize(1);
  tft.print(percentage);

  // GPU
  // GPU - Name
  tft.setTextSize(1);
  tft.setCursor(0, 50);
  tft.println(gpuName);

  // GPU - Temperature
  tft.setTextSize(2);
  tft.setCursor(1, 64);
  tft.print("GPU");
  tft.setCursor(40, 64);
  tft.print(gpuHotSpot.substring(0, gpuHotSpot.length() - 6));
  tft.setTextSize(1);
  tft.print(degree);

  // GPU - Load
  tft.setTextSize(2);
  tft.setCursor(86, 64);
  tft.print(gpuLoad.substring(0, gpuLoad.length() - 4));
  tft.setTextSize(1);
  tft.print(percentage);

  // RAM
  tft.setTextSize(1);
  tft.setCursor(0, 100);
  tft.println("Free RAM " + freeRAM);

  // RAM
  tft.setTextSize(1);
  tft.setCursor(0, 120);
  tft.println("Used RAM " + usedRAM);

  // Disconnect
  http.end();
}

void loop()
{
  // Update over Wi-Fi
  ArduinoOTA.handle();

  // Check WiFi Status
  if (WiFi.status() == WL_CONNECTED)
  {
    // Enable Wi-Fi Hardware Monitor with SSD1306 screen
    // hardwareMonitorSSD1306();
    // Enable Wi-Fi Hardware Monitor with ST7735 screen
    // hardwareMonitorST7735();
  }

  delay(1000);
}