#ifndef DEPARTURE_LIST_RETRIEVER_H
#define DEPARTURE_LIST_RETRIEVER_H

#include "types/TransitTypes.h"
#include "types/RouteList.h"
#include "types/StopList.h"
#include "types/DepartureList.h"
#include "backend/APICaller.h"
#include "backend/TimeRetriever.h"
#include "backend/DepartureRetriever.h"

/**
 * Fetches departures from MULTIPLE transitland stops for a SINGLE TransitZone
 */
class DepartureListRetriever
{
public:
  DepartureListRetriever(
      APICaller *caller,
      TimeRetriever *time,
      const DepartureRetrieverConfig &config);

  void init(RouteList routeList, StopList stopList);
  void clear();
  bool retrieve();

  DepartureList getDepartureList() const;

private:
  TimeRetriever *m_time;
  APICaller *m_caller;

  std::vector<Stop> m_stops;
  RouteList m_routeList;
  DepartureList m_departureList;
  DepartureRetrieverConfig m_config;
};

#endif