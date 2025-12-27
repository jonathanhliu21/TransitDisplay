#include "backend/TransitZone.h"

#include <Arduino.h>
#include <string>

#include "backend/TimeRetriever.h"
#include "backend/APICaller.h"
#include "backend/DepartureRetriever.h"
#include "backend/RouteRetriever.h"
#include "backend/StopRetriever.h"
#include "types/Whitelist.h"
#include "types/RouteList.h"
#include "types/StopList.h"
#include "types/DepartureList.h"

TransitZone::TransitZone(const std::string &name,
                         const float lat,
                         const float lon,
                         const float radius,
                         APICaller *caller,
                         TimeRetriever *time,
                         const DepartureRetrieverConfig &config)
    : m_name{name}, m_lat{lat}, m_lon{lon}, m_radius{radius},
      m_isValid{false}, m_isInitialized{false},
      m_caller{caller}, m_time{time},
      m_departureListRetriever{m_caller, m_time, config},
      m_status{TransitZoneStatus::UNINITIALIZED} {}

std::string TransitZone::getName() const { return m_name; }
bool TransitZone::isInitialized() const { return m_isInitialized; }
bool TransitZone::isValid() const { return m_isValid; }
float TransitZone::getLat() const { return m_lat; }
float TransitZone::getLon() const { return m_lon; }
float TransitZone::getRadius() const { return m_radius; }
RouteList TransitZone::getRoutes() const { return m_routeList; }
DepartureList TransitZone::getDepartures() const
{
  return m_departureListRetriever.getDepartureList();
}
Whitelist TransitZone::getWhitelist() const { return m_whitelist; }

void TransitZone::init()
{
  init(Whitelist());
}

/**
 * Re-initializes no matter what
 */
void TransitZone::init(const Whitelist &whitelist)
{
  clearDepartures();
  m_isInitialized = true;

  RouteRetriever routeRetriever{m_caller, m_lat, m_lon, m_radius, whitelist};
  StopRetriever stopRetriever{m_caller, m_lat, m_lon, m_radius, whitelist};

  m_status = TransitZoneStatus::RETRIEVING_ROUTES;
  if (routeRetriever.retrieve())
  {
    m_routeList = routeRetriever.getRouteList();
  }
  else
  {
    m_isValid = false;
    return;
  }

  m_status = TransitZoneStatus::RETRIEVING_STOPS;
  if (stopRetriever.retrieve())
  {
    m_stopList = stopRetriever.getStopList();
  }
  else
  {
    m_isValid = false;
    return;
  }

  m_departureListRetriever.init(m_routeList, m_stopList);
  m_isValid = !m_routeList.empty();

  m_status = TransitZoneStatus::IDLE;
  m_whitelist = whitelist;
}

void TransitZone::callDeparturesAPI()
{
  if (!isInitialized())
    return;

  m_status = TransitZoneStatus::RETRIEVING_DEPARTURES;
  m_departureListRetriever.retrieve();
  m_status = TransitZoneStatus::IDLE;
}

void TransitZone::clearDepartures()
{
  m_departureListRetriever.clear();
}

void TransitZone::debugPrint()
{
  Serial.print("---------- ");
  Serial.print(m_name.c_str());
  Serial.println(" ----------");

  Serial.println(isValid() ? "Valid Zone" : "Invalid Zone");
  Serial.println(isInitialized() ? "Initialized" : "Uninitialized");
  Serial.print(getLat(), 6);
  Serial.print(", ");
  Serial.print(getLon(), 6);
  Serial.print(" radius=");
  Serial.println(getRadius(), 6);

  getRoutes().debugPrintAllRoutes();
  getStops().debugPrintAllStops();
  getDepartures().debugPrintAllDepartures();
}

StopList TransitZone::getStops() const
{
  return m_stopList;
}