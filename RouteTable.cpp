#include "RouteTable.h"

#include <vector>

RouteTable::~RouteTable() {
  for (int i = 0; i < m_routes.size(); i++) {
    delete m_routes[i];
  }
}

Route *RouteTable::getRoute(const String &oneStopId) const {
  for (int i = 0; i < m_routes.size(); i++) {
    if (m_routes[i]->id == oneStopId) {
      return m_routes[i];
    }
  }

  return nullptr;
}

Route* RouteTable::addRoute(const Route &route) {
  // find if route exists
  Route *curRoute = getRoute(route.id);
  if (curRoute != nullptr) return curRoute;

  Route *newRoute = new Route;
  newRoute->id = route.id;
  newRoute->name = route.name;
  newRoute->lineColor = route.lineColor;
  newRoute->textColor = route.textColor;
  newRoute->agencyId = route.agencyId;
  
  m_routes.push_back(newRoute);
  return newRoute;
}