#ifndef TRANSIT_ZONE_H
#define TRANSIT_ZONE_H

#include <Arduino.h>
#include <ArduinoHttpClient.h>

#include "RouteTable.h"
#include "StopTable.h"
#include "Stop.h"

#include <vector>

class TransitZone {
public:
  TransitZone(String name, RouteTable *routeTable, StopTable *stopTable, HttpClient *client, const float lat, const float lon, const float radius);

  void init();

  String getName() const;
  bool getIsValidZone() const;

  void setWhiteList(std::vector<String> *whiteList);
  void clearWhiteList();

  void debugPrint() const;
private:
  String m_name;
  RouteTable *m_routeTable;
  StopTable *m_stopTable;
  HttpClient *m_client;
  float m_lat, m_lon;
  float m_radius;

  bool m_isValidZone;

  std::vector<Route*> m_routes;
  std::vector<Stop*> m_stops;

  std::vector<String> *m_whiteList;

  bool retrieveStops();
  String getWhiteListIds() const;
};

#endif