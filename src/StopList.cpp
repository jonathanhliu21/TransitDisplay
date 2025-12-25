#include "StopList.h"

#include <string>
#include "APICaller.h"

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

void StopList::addStop(const Stop &stop)
{
  m_stops[stop.onestopId] = stop;
}

void StopList::clear()
{
  m_stops.clear();
}