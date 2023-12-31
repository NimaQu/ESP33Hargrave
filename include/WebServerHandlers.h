#include <ESPAsyncWebServer.h>

#ifndef WEBSERVER_HANDLERS_H

void handleNotFound(AsyncWebServerRequest *request);

void handleTemperaturePost(AsyncWebServerRequest *request);

void handleTemperatureGet(AsyncWebServerRequest *request);

void handleFanPost(AsyncWebServerRequest *request);

void handleFanGet(AsyncWebServerRequest *request);

void handleWifiPost(AsyncWebServerRequest *request);

void handleWifiGet(AsyncWebServerRequest *request);


#endif