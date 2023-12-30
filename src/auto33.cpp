#include <Arduino.h>
#include <SPI.h>
#include "AD5270.h"
#include "config.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <EEPROM.h>
#include <Ticker.h>

void setTemperature(float temp);
void setFanMode(FanMode mode);
String responseJson(String code, String message, String data);
void printWiFiStatus();
void resetWiFi();
bool checkApiKeys(AsyncWebServerRequest *request);
void checkWifi();

Ticker wifiChecker;
AsyncWebServer server(80);
float currentTemperature = 24.0;
FanMode currentFanMode = FanMode::Auto;

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

  // setup EEPROM
  EEPROM.begin(512);

  // set up wifi checker
  wifiChecker.attach(30, checkWifi);

  // 按下超过 3 秒，重置 WiFi 设置
  pinMode(resetPin, INPUT_PULLUP);
  if (digitalRead(resetPin) == LOW)
  {
    delay(3000);
    if (digitalRead(resetPin) == LOW)
    {
      resetWiFi();
    }
  }

  // read wifi credential from EEPROM
  String ssidStored = EEPROM.readString(0);
  String passStored = EEPROM.readString(ssidStored.length() + 1);

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

  // Set up the web server
  server.on("/api/v1/temperature", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    if (!checkApiKeys(request)) {
      request->send(401, "application/json", responseJson("401", "invalid api key", "{}"));
      return;
    }

    if (request->hasParam("value", true)) {
      currentTemperature = request->getParam("value", true)->value().toFloat();
      setTemperature(currentTemperature);
      request->send(200, "application/json", responseJson("200", "success", "{}"));
    } else {
      request->send(400, "application/json", responseJson("400", "missing param: value", "{}"));
    } });

  server.on("/api/v1/temperature", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (!checkApiKeys(request)) {
      request->send(401, "application/json", responseJson("401", "invalid api key", "{}"));
      return;
    }

    String data = "{\"temperature\":" + String(currentTemperature) + "}";
    request->send(200, "application/json", responseJson("200", "success", data)); });

  server.on("/api/v1/fan", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    if (!checkApiKeys(request)) {
      request->send(401, "application/json", responseJson("401", "invalid api key", "{}"));
      return;
    }

    if (request->hasParam("mode", true)) {
      String fanMode = request->getParam("mode", true)->value();
      if (fanMode == "high") {
        currentFanMode = FanMode::High;
      } else if (fanMode == "med") {
        currentFanMode = FanMode::Med;
      } else if (fanMode == "low") {
        currentFanMode = FanMode::Low;
      } else if (fanMode == "auto") {
        currentFanMode = FanMode::Auto;
      } else if (fanMode == "off") {
        currentFanMode = FanMode::Off;
      } else {
        request->send(400, "application/json", responseJson("400", "invalid fan mode", "{}"));
        return;
      }
      setFanMode(currentFanMode);
      request->send(200, "application/json", responseJson("200", "success", "{}"));
    } else {
      request->send(400, "application/json", responseJson("400", "missing param: mode", "{}"));
    } });

  server.on("/api/v1/fan", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (!checkApiKeys(request)) {
      request->send(401, "application/json", responseJson("401", "invalid api key", "{}"));
      return;
    }

    String data = "";
    switch (currentFanMode){
      case FanMode::High:
        data = "{\"mode\":\"high\"}";
        break;
      case FanMode::Med:
        data = "{\"mode\":\"med\"}";
        break;
      case FanMode::Low:
        data = "{\"mode\":\"low\"}";
        break;
      case FanMode::Auto:
        data = "{\"mode\":\"auto\"}";
        break;
      case FanMode::Off:
        data = "{\"mode\":\"off\"}";
        break;
    };
    request->send(200, "application/json", responseJson("200", "success", data)); });

  server.on("/api/v1/wifi", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (!checkApiKeys(request)) {
      request->send(401, "application/json", responseJson("401", "invalid api key", "{}"));
      return;
    }

    String data = "{\"ssid\":\"" + String(WiFi.SSID()) + "\", \"ip\":\"" + String(WiFi.localIP()) + "\", \"rssi\":\"" + String(WiFi.RSSI()) + "\"}";
    request->send(200, "application/json", responseJson("200", "success", data)); });

  server.on("/api/v1/wifi", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    if (!checkApiKeys(request)) {
      request->send(401, "application/json", responseJson("401", "invalid api key", "{}"));
      return;
    }

    if (request->hasParam("ssid", true) && request->hasParam("pass", true)) {
      String ssid = request->getParam("ssid", true)->value();
      String pass = request->getParam("pass", true)->value();
      EEPROM.writeString(0, ssid);
      EEPROM.writeString(ssid.length() + 1, pass);
      EEPROM.commit();
      request->send(200, "application/json", responseJson("200", "success", "{}"));
      delay(3000);
      Serial.println("Restarting...");
      ESP.restart();
    } else {
      request->send(400, "application/json", responseJson("400", "missing param: ssid or pass", "{}"));
    } });

  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404, "application/json", responseJson("404", "not found", "{}")); });

  // 静态文件
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  // 启动服务器
  server.begin();
}

void loop()
{
}

void setTemperature(float temp)
{
  float res = calcResistanceFromTemperature(temp);
  setResistance(tempCsPinCWF, res);
}

void setFanMode(FanMode mode)
{
  setResistance(fanCsPinCWF, FanResistances[mode]);
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

String responseJson(String code, String message, String data)
{
  return "{\"code\":\"" + code + "\", \"message\":\"" + message + "\", \"data\":" + data + "}";
}

bool checkApiKeys(AsyncWebServerRequest *request)
{
  // 从 header 中获取 api key
  String apiKey = request->header("api-key");
  // 检查 api key 是否有效
  for (int i = 0; i < sizeof(apiKeys) / sizeof(apiKeys[0]); i++)
  {
    if (apiKey == apiKeys[i])
    {
      return true;
    }
  }
  return false;
}

void resetWiFi()
{
  bool isEmpty = true;

  // 检查 EEPROM 是否为空（所有位置都为零）
  for (int i = 0; i < 512; ++i)
  {
    if (EEPROM.read(i) != 0)
    {
      isEmpty = false;
      break; // 找到非零值，跳出循环
    }
  }

  // 如果 EEPROM 已经为空，则跳过重置
  if (isEmpty)
  {
    Serial.println("WiFi settings already reset, skipping...");
    return;
  }

  // 清空 EEPROM
  for (int i = 0; i < 512; ++i)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();

  Serial.println("WiFi settings reset. Please reconfigure.");
  ESP.restart();
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