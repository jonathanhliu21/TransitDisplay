#ifndef DEPARTURES_LIST_H
#define DEPARTURES_LIST_H

#include <vector>
#include <map>
#include <ctime>
#include "types/TransitTypes.h"
#include "types/DisplayTypes.h"

class DepartureList
{
public:
  DepartureList(const int numStored = -1);

  bool empty() const;
  int size() const;
  std::vector<Departure> getDepartures() const;
  std::vector<DisplayDeparture> getDisplayDepartureList(
      const std::time_t curTime,
      const int onTimeColor,
      const int delayedColor,
      const int earlyColor,
      const int noRtInfoColor,
      const int delayCutoff) const;

  void addDeparture(const Departure &departure);
  void concat(const DepartureList &other);
  void removeAllBefore(const std::time_t time);
  void shrinkTo(const int size);
  void clear();

  void debugPrintAllDepartures() const;

private:
  int m_numStored;
  std::multimap<std::time_t, Departure> m_departures;

  int getDelayColor(const int delay, const bool isRealTime,
                    const int onTimeColor,
                    const int delayedColor,
                    const int earlyColor,
                    const int noRtInfoColor,
                    const int delayCutoff) const;
};

#endif