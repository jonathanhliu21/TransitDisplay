#ifndef TRANSIT_ZONE_H
#define TRANSIT_ZONE_H

#include <Arduino.h>
#include <HTTPClient.h>

#include "RouteTable.h"
#include "StopTable.h"
#include "Stop.h"

#include <ctime>
#include <vector>

class TransitZone {
public:
  TransitZone(String name, RouteTable *routeTable, StopTable *stopTable, const float lat, const float lon, const float radius);

  void init();

  String getName() const;
  bool getIsValidZone() const;
  std::vector<Route *> getRoutes() const;
  std::vector<Departure> getDepartures() const;

  void setWhiteList(std::vector<String> *whiteList);
  void clearWhiteList();
  void updateDepartures(std::time_t curTime);

  void debugPrint() const;
private:
  String m_name;
  RouteTable *m_routeTable;
  StopTable *m_stopTable;
  float m_lat, m_lon;
  float m_radius;

  bool m_isValidZone;

  std::vector<Route*> m_routes;
  std::vector<Stop*> m_stops;
  std::vector<Departure> m_departures;

  std::vector<String> *m_whiteList;

  bool retrieveStops();
  String getWhiteListIds() const;
  void combineDepartures(std::vector<Departure> &a, const std::vector<Departure> &b);
  void checkDepTimes(std::vector<Departure> &deps, std::time_t curTime);
};

#endif