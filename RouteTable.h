#ifndef ROUTE_TABLE_H
#define ROUTE_TABLE_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include <cstdint>
#include <vector>

struct Route {
  String id;
  String name;
  uint32_t lineColor; // hex
  uint32_t textColor;
  String agencyId;
};

class RouteTable {
public:
  RouteTable() = default;
  ~RouteTable();
  
  std::vector<Route*> retrieveRoutes(float lat, float lon, float radius, std::vector<String> *whiteList = nullptr);
  Route* getRoute(const String &oneStopId) const;

  void debugPrintAllRoutes() const;

private:
  
  // const std::vector<String> *m_supportedAgencies;
  std::vector<Route*> m_routes;

  Route* addRoute(const Route &route);
};

#endif