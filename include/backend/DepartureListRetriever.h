#ifndef DEPARTURE_LIST_RETRIEVER_H
#define DEPARTURE_LIST_RETRIEVER_H

#include "types/TransitTypes.h"
#include "types/RouteList.h"
#include "types/StopList.h"
#include "types/DepartureList.h"
#include "backend/APICaller.h"
#include "backend/TimeRetriever.h"

/**
 * Fetches departures from MULTIPLE transitland stops for a SINGLE TransitZone
 */
class DepartureListRetriever
{
public:
  DepartureListRetriever(
      APICaller *caller,
      TimeRetriever *time,
      const int departureLimit,
      const int nextNSeconds,
      const int timestampCutoff);

  void init(RouteList routeList, StopList stopList);
  bool retrieve();

  DepartureList getDepartureList();

private:
  TimeRetriever *m_time;
  APICaller *m_caller;

  std::vector<Stop> m_stops;
  RouteList m_routeList;
  DepartureList m_departureList;
  int m_departureLimit;
  int m_nextNSeconds;
  int m_timestampCutoff;
};

#endif