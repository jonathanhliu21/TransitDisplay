#ifndef STOP_TABLE_H
#define STOP_TABLE_H

#include <Arduino.h>
#include <vector>

#include "Stop.h"

class StopTable {
public:
  StopTable() = default;
  ~StopTable();

  Stop *addStop(const Stop &stop);
  Stop *getStop(const String &oneStopId) const;

  void debugPrintAllStops() const;

private:
  std::vector<Stop*> m_stops;
};

#endif