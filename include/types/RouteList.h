#ifndef ROUTE_LIST_H
#define ROUTE_LIST_H

#include <string>
#include <unordered_map>

#include "types/TransitTypes.h"

class RouteList
{
public:
  RouteList() = default;

  bool routeExists(const std::string &onestopId) const;
  Route getRoute(const std::string &onestopId) const;
  bool empty() const;
  int size() const;

  void addRoute(const Route &route);
  void clear();

  void debugPrintAllRoutes() const;

private:
  std::unordered_map<std::string, Route> m_routes; // unordered_map does not work
};

#endif