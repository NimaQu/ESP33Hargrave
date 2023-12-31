#include "WebServerHandlers.h"
#include "Config.h"
#include "deviceControl.h"
#include <Preferences.h>

extern float currentTemperature;
extern FanMode currentFanMode;
extern Preferences preferences;
bool checkApiKeys(AsyncWebServerRequest *request);
void resetWifiSetting();
String responseJson(String code, String message, String data);


// not found
void handleNotFound(AsyncWebServerRequest *request)
{
    request->send(404, "application/json", responseJson("404", "not found", "{}"));
}

// Post /api/v1/temperature
void handleTemperaturePost(AsyncWebServerRequest *request)
{
    if (!checkApiKeys(request))
    {
        request->send(401, "application/json", responseJson("401", "invalid api key", "{}"));
        return;
    }

    if (request->hasParam("value", true))
    {
        currentTemperature = request->getParam("value", true)->value().toFloat();
        setTemperature(currentTemperature);
        request->send(200, "application/json", responseJson("200", "success", "{}"));
    }
    else
    {
        request->send(400, "application/json", responseJson("400", "missing param: value", "{}"));
    }
}

// Get /api/v1/temperature
void handleTemperatureGet(AsyncWebServerRequest *request)
{
    if (!checkApiKeys(request))
    {
        request->send(401, "application/json", responseJson("401", "invalid api key", "{}"));
        return;
    }

    String data = "{\"temperature\":" + String(currentTemperature) + "}";
    request->send(200, "application/json", responseJson("200", "success", data));
}

// Post /api/v1/fan
void handleFanPost(AsyncWebServerRequest *request)
{
    if (!checkApiKeys(request))
    {
        request->send(401, "application/json", responseJson("401", "invalid api key", "{}"));
        return;
    }

    if (request->hasParam("mode", true))
    {
        String fanMode = request->getParam("mode", true)->value();
        if (fanMode == "high")
        {
            currentFanMode = FanMode::High;
        }
        else if (fanMode == "med")
        {
            currentFanMode = FanMode::Med;
        }
        else if (fanMode == "low")
        {
            currentFanMode = FanMode::Low;
        }
        else if (fanMode == "auto")
        {
            currentFanMode = FanMode::Auto;
        }
        else if (fanMode == "off")
        {
            currentFanMode = FanMode::Off;
        }
        else
        {
            request->send(400, "application/json", responseJson("400", "invalid fan mode", "{}"));
            return;
        }
        setFanMode(currentFanMode);
        request->send(200, "application/json", responseJson("200", "success", "{}"));
    }
    else
    {
        request->send(400, "application/json", responseJson("400", "missing param: mode", "{}"));
    }
}

// Get /api/v1/fan
void handleFanGet(AsyncWebServerRequest *request)
{
    if (!checkApiKeys(request))
    {
        request->send(401, "application/json", responseJson("401", "invalid api key", "{}"));
        return;
    }

    String data = "";
    switch (currentFanMode)
    {
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
    request->send(200, "application/json", responseJson("200", "success", data));
}

// Post /api/v1/wifi
void handleWifiPost(AsyncWebServerRequest *request)
{
    if (!checkApiKeys(request))
    {
        request->send(401, "application/json", responseJson("401", "invalid api key", "{}"));
        return;
    }

    if (request->hasParam("ssid", true) && request->hasParam("pass", true))
    {
        String ssid = request->getParam("ssid", true)->value();
        String pass = request->getParam("pass", true)->value();
        preferences.begin("ESP33Hargrave", false);
        preferences.putString("ssid", ssid);
        preferences.putString("pass", pass);
        preferences.end();
        request->send(200, "application/json", responseJson("200", "success", "{}"));
        delay(3000);
        Serial.println("Restarting...");
        ESP.restart();
    }
    else
    {
        request->send(400, "application/json", responseJson("400", "missing param: ssid or pass", "{}"));
    }
}

// Get /api/v1/wifi
void handleWifiGet(AsyncWebServerRequest *request)
{
    if (!checkApiKeys(request))
    {
        request->send(401, "application/json", responseJson("401", "invalid api key", "{}"));
        return;
    }

    String data = "{\"ssid\":\"" + String(WiFi.SSID()) + "\", \"ip\":\"" + String(WiFi.localIP()) + "\", \"rssi\":\"" + String(WiFi.RSSI()) + "\"}";
    request->send(200, "application/json", responseJson("200", "success", data));
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