#ifndef AD5270_H
#define AD5270_H

#include <Arduino.h>
#include <SPI.h>

void enablePotWrite(int csPin);
void setResistance(int csPin, float resistance);
void digitalPotWrite(int csPin, int value);
void beginTransaction(int csPin);
void endTransaction(int csPin);
float calcResistanceFromTemperature(float temp);

const int nominalResistance = 20000; // 20k ohms
const int maxSteps = 1024;           // 10 bit resolution

const byte enableUpdateMSB = 0x1C; // B00011100
const byte enableUpdateLSB = 0x02; // B00000010
const byte command = 0x04;         // B00000100
const int resetPin = 8;

#endif // AD5270_H
