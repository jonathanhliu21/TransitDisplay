#ifndef TRANSIT_ZONE_H
#define TRANSIT_ZONE_H

#include <atomic>
#include <string>

#include "backend/TimeRetriever.h"
#include "backend/APICaller.h"
#include "backend/DepartureListRetriever.h"
#include "types/Whitelist.h"
#include "types/RouteList.h"
#include "types/StopList.h"
#include "types/DepartureList.h"

enum class TransitZoneStatus
{
  UNINITIALIZED,
  RETRIEVING_ROUTES,
  RETRIEVING_STOPS,
  RETRIEVING_DEPARTURES,
  IDLE
};

class TransitZone
{
public:
  TransitZone(const std::string &name,
              const float lat,
              const float lon,
              const float radius,
              APICaller *caller,
              TimeRetriever *time,
              const DepartureRetrieverConfig &config);

  std::string getName() const;
  float getLat() const;
  float getLon() const;
  float getRadius() const;
  bool isInitialized() const;
  bool isValid() const;

  RouteList getRoutes() const;
  DepartureList getDepartures() const;
  TransitZoneStatus getStatus() const;

  void init();
  void init(const Whitelist &whitelist);
  void callDeparturesAPI();
  void clearDepartures();

  void debugPrint();

private:
  std::string m_name;
  float m_lat, m_lon, m_radius;
  bool m_isValid;
  bool m_isInitialized;
  std::atomic<TransitZoneStatus> m_status;

  APICaller *m_caller;
  TimeRetriever *m_time;

  RouteList m_routeList;
  StopList m_stopList;
  DepartureListRetriever m_departureListRetriever;

  StopList getStops() const;
};

#endif