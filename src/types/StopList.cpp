#include "types/StopList.h"

#include <Arduino.h>
#include <string>

StopList::StopList() {}

bool StopList::stopExists(const std::string &onestopId) const
{
  return m_stops.find(onestopId) != m_stops.end();
}

Stop StopList::getStop(const std::string &onestopId) const
{
  auto it = m_stops.find(onestopId);
  if (it == m_stops.end())
  {
    return {};
  }
  return it->second;
}

std::vector<Stop> StopList::getAllStops() const
{
  std::vector<Stop> res;
  for (const auto &stop : m_stops)
  {
    res.push_back(stop.second);
  }
  return res;
}

bool StopList::empty() const
{
  return m_stops.empty();
}

int StopList::size() const
{
  return m_stops.size();
}

void StopList::addStop(const Stop &stop)
{
  m_stops[stop.onestopId] = stop;
}

void StopList::clear()
{
  m_stops.clear();
}

void StopList::debugPrintAllStops() const
{
  Serial.println(F("--- Stop Info ---"));
  if (m_stops.size() == 0)
  {
    Serial.println("No stops found");
    return;
  }

  for (const auto &s : m_stops)
  {
    const auto &stop = s.second;
    Serial.print(F("ID: "));
    Serial.println(stop.onestopId.c_str());

    Serial.print(F("Name: "));
    Serial.println(stop.name.c_str());

    Serial.println(F("------------------"));
  }
}
