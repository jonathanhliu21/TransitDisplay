#ifndef BASE_DISPLAYER_H
#define BASE_DISPLAYER_H

#include <Arduino.h>

class BaseDisplayer
{
public:
  static uint16_t hexToRGB565(uint32_t hexColor);
  virtual void cycle() = 0;

private:
};

#endif