#ifndef ROUTE_TABLE_H
#define ROUTE_TABLE_H

#include <Arduino.h>
#include <ArduinoHttpClient.h>

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
  RouteTable(HttpClient *client);
  ~RouteTable();
  
  std::vector<Route*> retrieveRoutes(float lat, float lon, float radius, std::vector<String> *whiteList = nullptr);
  Route* getRoute(const String &oneStopId) const;

  void debugPrintAllRoutes() const;

private:
  HttpClient *m_client;
  // const std::vector<String> *m_supportedAgencies;
  std::vector<Route*> m_routes;

  Route* addRoute(const Route &route);

  void modifyRoute(Route &route) const;
  String truncateRoute(const String &routeStr) const;
};

#endif