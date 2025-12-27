#include "types/DepartureList.h"

#include <Arduino.h>
#include <string>
#include <vector>

DepartureList::DepartureList(const int numStored) : m_numStored{numStored} {}

bool DepartureList::empty() const
{
  return m_departures.empty();
}

int DepartureList::size() const
{
  return m_departures.size();
}

/**
 * Gets all departures
 *
 * Assume that the departures are sorted
 */
std::vector<Departure> DepartureList::getDepartures() const
{
  std::vector<Departure> res;
  for (const auto &dep : m_departures)
  {
    res.push_back(dep.second);
  }

  return res;
}

void DepartureList::addDeparture(const Departure &departure)
{
  m_departures.insert({departure.actualTimestamp, departure});
  if (m_numStored >= 0 && m_departures.size() > m_numStored)
  {
    // remove the greatest timestamp because it won't be shown
    m_departures.erase(std::prev(m_departures.end()));
  }
}

void DepartureList::concat(const DepartureList &other)
{
  for (const auto &dep : other.m_departures)
  {
    addDeparture(dep.second);
  }
}

void DepartureList::removeAllBefore(const std::time_t time)
{
  m_departures.erase(m_departures.begin(), m_departures.lower_bound(time));
}

void DepartureList::shrinkTo(const int size)
{
  if (m_departures.size() <= size)
    return;
  auto it = std::next(m_departures.begin(), size);
  m_departures.erase(it, m_departures.end());
}

void DepartureList::clear()
{
  m_departures.clear();
}

void DepartureList::debugPrintAllDepartures() const
{
  Serial.println(F("--- Departure Info ---"));
  if (m_departures.size() == 0)
  {
    Serial.println("No departures found");
    return;
  }

  for (const auto &d : m_departures)
  {
    Serial.println(F("    ----------------------"));

    const auto &dep = d.second;
    Serial.print(F("    Stop Name: "));
    Serial.println(dep.stop.name.c_str());
    Serial.print(F("    Route Name: "));
    Serial.println(dep.route.name.c_str());
    Serial.print(F("    Direction: "));
    Serial.println(dep.direction.c_str());
    Serial.print(F("    Is Real-Time: "));
    Serial.println(dep.isRealTime ? "Yes" : "No");
    Serial.print(F("    Agency: "));
    Serial.println(dep.agencyOnestopId.c_str());
    Serial.print(F("    Exp timestamp: "));
    Serial.println(dep.expectedTimestamp);
    Serial.print(F("    Act timestamp: "));
    Serial.println(dep.actualTimestamp);
    Serial.print(F("    Delay (seconds): "));
    Serial.println(dep.delay);
    Serial.print(F("    Is Valid "));
    Serial.println(dep.isValid ? "Yes" : "No");
  }
  Serial.println(F("    ----------------------"));
}