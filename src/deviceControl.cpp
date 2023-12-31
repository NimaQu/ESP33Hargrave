#include "deviceControl.h"

void setTemperature(float temp)
{
  float res = calcResistanceFromTemperature(temp);
  setResistance(tempCsPinCWF, res);
}

void setFanMode(FanMode mode)
{
  setResistance(fanCsPinCWF, FanResistances[mode]);
}