#include "deviceControl.h"

extern int tempResOffsetStored;

void setTemperature(float temp)
{
  float res = calcResistanceFromTemperature(temp);
  res += tempResOffsetStored;
  Serial.println("Setting temperature to " + String(temp) + "C, resistance: " + String(res) + "Ohm");
  setResistance(tempCsPinCWF, res);
}

void setFanMode(FanMode mode)
{
  setResistance(fanCsPinCWF, FanResistances[mode]);
}