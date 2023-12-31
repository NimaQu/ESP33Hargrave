#include <Arduino.h>
#include <SPI.h>
#include "AD5270.h"
#include "config.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include "WebServerHandlers.h"
#include <Ticker.h>

void setTemperature(float temp);
void setFanMode(FanMode mode);
String responseJson(String code, String message, String data);
void printWiFiStatus();
void resetWifiSetting();
bool checkApiKeys(AsyncWebServerRequest *request);
void checkWifi();

Ticker wifiChecker;
AsyncWebServer server(80);
float currentTemperature = 24.0;
FanMode currentFanMode = FanMode::Auto;
Preferences preferences;

void setup()
{
  Serial.begin(115200);
  SPI.begin();

  // Set up the chip select pins
  pinMode(fanCsPinCWF, OUTPUT);
  digitalWrite(fanCsPinCWF, HIGH);

  pinMode(tempCsPinCWF, OUTPUT);
  digitalWrite(tempCsPinCWF, HIGH);

  // Set up the potentiometers
  enablePotWrite(fanCsPinCWF);
  enablePotWrite(tempCsPinCWF);

  // Set the potentiometers to their default values
  setFanMode(FanMode::Auto);
  setTemperature(24.0);

  // setup SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // setup nvs
  if (!preferences.begin("ESP33Hargrave", false))
  {
    Serial.println("An Error has occurred while mounting nvs");
    return;
  }

  // set up wifi checker
  wifiChecker.attach(30, checkWifi);

  // 按下超过 3 秒，重置 WiFi 设置
  pinMode(resetPin, INPUT_PULLUP);
  if (digitalRead(resetPin) == LOW)
  {
    delay(3000);
    if (digitalRead(resetPin) == LOW)
    {
      if (preferences.getString("ssid", "").length() != 0 && preferences.getString("pass", "").length() != 0)
      {
        preferences.remove("ssid");
        preferences.remove("pass");
        preferences.end();
        Serial.println("WiFi settings reset, restarting...");
        ESP.restart();
      }
    }
  }

  // read settings from nvs
  String ssidStored = preferences.getString("ssid", "");
  String passStored = preferences.getString("pass", "");

  int resOffsetStored = preferences.getInt("resOffset", 0);

  if (ssidStored.length() > 0)
  {
    WiFi.begin(ssidStored.c_str(), passStored.c_str());
    Serial.println("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
  }
  else
  {
    Serial.println("No WiFi credentials stored, starting AP for configuration...");
    WiFi.softAP("Fubuki Dev - ESP33Hargrave", "1145141919");
  }

  printWiFiStatus();
  preferences.end();

  // Set up the web server
  server.on("/api/v1/temperature", HTTP_POST, handleTemperaturePost);

  server.on("/api/v1/temperature", HTTP_GET, handleTemperatureGet);

  server.on("/api/v1/fan", HTTP_POST, handleFanPost);

  server.on("/api/v1/fan", HTTP_GET, handleFanGet);

  server.on("/api/v1/wifi", HTTP_GET, handleWifiGet);

  server.on("/api/v1/wifi", HTTP_POST, handleWifiPost);

  server.onNotFound(handleNotFound);

  // 静态文件
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  // 启动服务器
  server.begin();
}

void loop()
{
}

void printWiFiStatus()
{
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void checkWifi()
{
  // 检查当前 WiFi 模式
  if (WiFi.getMode() != WIFI_STA)
  {
    return;
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi disconnected, attempting reconnection...");
    WiFi.reconnect();
  }
}