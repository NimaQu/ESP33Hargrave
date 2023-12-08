#include "AD5270.h"

float calcResistanceFromTemperature(float temp)
{
  float tempMax = 29.4;
  float tempMin = 12.8;
  float tempMaxResistance = 208;
  float tempMinResistance = 792;

  if (temp > tempMax)
  {
    temp = tempMax;
  }
  else if (temp < tempMin)
  {
    temp = tempMin;
  }

  float resistance = ((temp - tempMin) / (tempMax - tempMin)) * (tempMaxResistance - tempMinResistance) + tempMinResistance;
  return resistance;
}

void setResistance(int csPin, float resistance)
{
  // Based on the value of the potentiometer, set the resistance
  int value = round((resistance * maxSteps) / (float)nominalResistance);
  if (value > maxSteps - 1)
  {
    Serial.print("resistance is greater than maximum: ");
    Serial.print(resistance);
    value = maxSteps - 1;
  }
  else if (value < 0)
  {
    Serial.print("resistance is less than minimum: ");
    Serial.print(resistance);
    value = 0;
  }
  digitalPotWrite(csPin, value);
}

void digitalPotWrite(int csPin, int value)
{
  beginTransaction(csPin);

  byte shfitedValue = (value >> 8);
  byte byte1 = (command | shfitedValue);
  byte byte0 = (value & 0xFF); // 0xFF = B11111111 trunicates value to 8 bits

  SPI.transfer(byte1);
  SPI.transfer(byte0);

  endTransaction(csPin);
}

void enablePotWrite(int csPin)
{
  beginTransaction(csPin);

  SPI.transfer(enableUpdateMSB);
  SPI.transfer(enableUpdateLSB);

  endTransaction(csPin);
}

void beginTransaction(int csPin)
{
  digitalWrite(csPin, LOW); // select slave
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
}

void endTransaction(int csPin)
{
  SPI.endTransaction();
  digitalWrite(csPin, HIGH); // de-select slave
}