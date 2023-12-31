#include <Arduino.h>

#pragma once
#ifndef CONFIG_H
#define CONFIG_H

const int fanCsPinCWF = 9;
const int tempCsPinCWF = 10;

const char ssid[] = "Fubuki Networks";
const char pass[] = "1145141919";
const String apiKeys[] = {"1145141919"};

enum FanMode
{
  High,
  Med,
  Low,
  Auto,
  Off,
  FanModeCount
};

const int FanResistances[FanModeCount] = {16130, 13320, 10770, 2320, 4870};

#endif
