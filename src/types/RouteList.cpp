#include "types/RouteList.h"

#include <string>
#include <Arduino.h>

bool RouteList::routeExists(const std::string &onestopId) const
{
  return m_routes.find(onestopId) != m_routes.end();
}

Route RouteList::getRoute(const std::string &onestopId) const
{
  auto it = m_routes.find(onestopId);
  if (it == m_routes.end())
  {
    return {};
  }
  return it->second;
}

bool RouteList::empty() const
{
  return m_routes.empty();
}

int RouteList::size() const
{
  return m_routes.size();
}

void RouteList::addRoute(const Route &route)
{
  m_routes[route.onestopId] = route;
}

void RouteList::clear()
{
  m_routes.clear();
}

void RouteList::debugPrintAllRoutes() const
{
  Serial.println(F("--- Route Info ---"));
  if (m_routes.size() == 0)
  {
    Serial.println("No routes found");
    return;
  }

  for (const auto &r : m_routes)
  {
    const auto &route = r.second;
    Serial.print(F("ID: "));
    Serial.println(route.onestopId.c_str());

    Serial.print(F("Name: "));
    Serial.println(route.name.c_str());

    Serial.print(F("Line Color: 0x"));
    Serial.println(route.lineColor, HEX);

    Serial.print(F("Text Color: 0x"));
    Serial.println(route.textColor, HEX);

    Serial.print(F("Agency ID: "));
    Serial.println(route.agencyOnestopId.c_str());

    Serial.println(F("------------------"));
  }
}