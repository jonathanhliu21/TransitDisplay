#ifndef DEPARTURES_LIST_H
#define DEPARTURES_LIST_H

#include <vector>
#include <map>
#include <ctime>
#include "types/TransitTypes.h"

class DepartureList
{
public:
  DepartureList(const int numStored = -1);

  bool empty() const;
  int size() const;
  std::vector<Departure> getDepartures() const;

  void addDeparture(const Departure &departure);
  void concat(const DepartureList &other);
  void removeAllBefore(const std::time_t time);
  void shrinkTo(const int size);
  void clear();

  void debugPrintAllDepartures() const;

private:
  int m_numStored;
  std::multimap<std::time_t, Departure> m_departures;
};

#endif