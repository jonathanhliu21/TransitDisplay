#include "backend/DepartureListRetriever.h"

#include "backend/APICaller.h"
#include "backend/TimeRetriever.h"
#include "backend/DepartureRetriever.h"

DepartureListRetriever::DepartureListRetriever(APICaller *caller,
                                               TimeRetriever *time,
                                               const int departureLimit,
                                               const int nextNSeconds,
                                               const int timestampCutoff)
    : m_time{time}, m_caller{caller}, m_departureList{departureLimit}, m_departureLimit{departureLimit}, m_nextNSeconds{nextNSeconds}, m_timestampCutoff{timestampCutoff} {}

void DepartureListRetriever::init(RouteList routeList, StopList stopList)
{
  m_routeList = routeList;
  m_stops = stopList.getAllStops();
}

/**
 * Returns false if AT LEAST ONE departure went wrong
 */
bool DepartureListRetriever::retrieve()
{
  m_departureList.clear();

  bool res = true;
  for (const Stop &stop : m_stops)
  {
    DepartureRetriever depRetriever(m_caller,
                                    m_time,
                                    stop,
                                    m_routeList,
                                    m_departureLimit,
                                    m_nextNSeconds,
                                    m_timestampCutoff);

    if (depRetriever.retrieve())
    {
      m_departureList.concat(depRetriever.getDepartureList());
    }
    else
    {
      // don't fail - we still may want other departures as well
      res = false;
    }
  }

  return res;
}

DepartureList DepartureListRetriever::getDepartureList()
{
  return m_departureList;
}