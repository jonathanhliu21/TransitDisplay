#ifndef ROUTE_TABLE_H
#define ROUTE_TABLE_H

#include <Arduino.h>

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
  
  Route* getRoute(const String &oneStopId) const;
  Route* addRoute(const Route &route);

private:
  std::vector<Route*> m_routes;

  Route modifyRoute(const Route &route) const;
};

#endif