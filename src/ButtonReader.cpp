#include "ButtonReader.h"

namespace
{
  const int DEFAULT_DEBOUNCE_DELAY = 50; // ms
}

ButtonReader::ButtonReader(const int pin) : ButtonReader{pin, DEFAULT_DEBOUNCE_DELAY} {}

ButtonReader::ButtonReader(const int pin, const unsigned long debounceDelay)
    : m_pin{pin}, m_lastState{0}, m_lastDebounceState{0}, m_lastDebounceTime{0}, m_debounceDelay{debounceDelay} {}

bool ButtonReader::readButton()
{
  int curState = digitalRead(m_pin);
  bool res = false;

  if (curState != m_lastState)
  {
    m_lastDebounceTime = millis();
  }

  if (millis() - m_lastDebounceTime > m_debounceDelay)
  {
    // we can "trust" curState to be stable
    if (curState == LOW && m_lastDebounceState == HIGH)
    {
      res = true;
    }
    m_lastDebounceState = curState; // only assign m_lastDebounceState to trustable state
  }
  m_lastState = curState;
  return res;
}
