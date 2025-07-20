#ifndef BRIDGE_H
#define BRIDGE_H

#include <vector>

#include "TransitZone.h"
#include "RouteTable.h"

struct BridgeDeparture {
  String direction;
  int mins;
  String line;
  int textColor;
  int routeColor;
  int delayColor;
};

// Bridges the code between calling the API and updating the display
class Bridge {
public:
  Bridge() = default;
  void setZone(TransitZone *zone);
  void setTimeSync(time_t startTimeMillis, time_t startTimeUTC);
  void retrieveDepartures();

  void debugPrintRoutes() const;
  void debugPrintDepartures() const;

private:
  TransitZone *m_zone;
  String m_name;
  std::vector<Route> m_routes;
  std::vector<BridgeDeparture> m_departures;
  time_t m_startTimeS, m_startTimeUTC;
  bool m_isCurrentlyRetrieving = false;

  time_t retrieveTime() const;
  int getDelayColor(int delay, bool isRealTime) const;

  void modifyRoutes();
  Departure modifyDeparture(const Departure &dep);
  String truncateStop(const String &stopName, const bool truncateDowntown) const;
  String truncateRoute(const String &routeName) const;
  void sortRoutes(std::vector<Route *> &routes) const;
};

#endif