#include "StopTable.h"

#include "Stop.h"

StopTable::~StopTable() {
  for (int i = 0; i < m_stops.size(); i++) {
    if (m_stops[i] != nullptr) {
      delete m_stops[i];
      m_stops[i] = nullptr;
    }
  }
}

Stop *StopTable::addStop(const Stop &stop) {
  // find if stop exists
  Stop *curStop = getStop(stop.getId());
  if (curStop != nullptr) return curStop;

  Stop *newStop = new Stop(stop);
  m_stops.push_back(newStop);

  return newStop;
}

Stop *StopTable::getStop(const String &oneStopId) const {
  for (int i = 0; i < m_stops.size(); i++) {
    if (m_stops[i]->getId() == oneStopId) {
      return m_stops[i];
    }
  }

  return nullptr;
}

void StopTable::debugPrintAllStops() const {
  Serial.println(F("--- Stop Info ---"));
  if (m_stops.size() == 0) {
    Serial.println("No stops found");
    return;
  }

  for (int i = 0; i < m_stops.size(); i++) {
    m_stops[i]->debugPrintStop();
  }
}