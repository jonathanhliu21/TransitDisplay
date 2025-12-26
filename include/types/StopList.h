#ifndef STOP_LIST_H
#define STOP_LIST_H

#include <vector>
#include <string>
#include <unordered_map>

#include "types/TransitTypes.h"

class StopList
{
public:
  StopList();

  bool stopExists(const std::string &onestopId) const;
  Stop getStop(const std::string &onestopId) const;
  std::vector<Stop> getAllStops() const;
  bool empty() const;

  void addStop(const Stop &stop);
  void clear();

  void debugPrintAllStops() const;

private:
  std::unordered_map<std::string, Stop> m_stops;
};

#endif