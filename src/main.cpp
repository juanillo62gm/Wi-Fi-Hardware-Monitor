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

// You need to add the followind data to inside /include/secrets.h
// const char *ssid = "YOUR_SSID";
// const char *password = "YOUR_PASSWORD";
// const char *url = "YOUR_OPENHARDWAREMONITOR_SERVER_URL";
#include "secrets.h"

// Screen
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);

void setup()
{
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  while (WiFi.status() != WL_CONNECTED)
  {
    // Console Log
    Serial.println("Connecting to Wi-Fi");
    delay(1000);
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
}

void loop()
{
  // Check WiFi Status
  if (WiFi.status() == WL_CONNECTED)
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
    Serial.println(gpuLoad);
    Serial.print("Free RAM: ");
    Serial.println(gpuLoad);

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

    // Delay
    delay(1000);
  }
}