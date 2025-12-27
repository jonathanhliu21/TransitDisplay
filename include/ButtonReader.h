#ifndef BUTTON_READER_H
#define BUTTON_READER_H

#include <Arduino.h>

class ButtonReader
{
public:
  ButtonReader(const int pin);
  ButtonReader(const int pin, const unsigned long debounceDelay);

  bool readButton(); // should be called in loop
private:
  int m_pin;
  bool m_lastState;
  bool m_lastDebounceState;
  unsigned long m_lastDebounceTime;
  unsigned long m_debounceDelay; // ms
};

#endif