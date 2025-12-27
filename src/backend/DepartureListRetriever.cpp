#include "backend/DepartureListRetriever.h"

#include "backend/APICaller.h"
#include "backend/TimeRetriever.h"
#include "backend/DepartureRetriever.h"

DepartureListRetriever::DepartureListRetriever(APICaller *caller,
                                               TimeRetriever *time,
                                               const DepartureRetrieverConfig &config)
    : m_time{time}, m_caller{caller}, m_departureList{config.departureLimit}, m_config{config} {}

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
  clear();

  bool res = true;
  for (const Stop &stop : m_stops)
  {
    DepartureRetriever depRetriever(m_caller,
                                    m_time,
                                    stop,
                                    m_routeList,
                                    m_config);

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

void DepartureListRetriever::clear()
{
  m_departureList.clear();
}

DepartureList DepartureListRetriever::getDepartureList() const
{
  return m_departureList;
}