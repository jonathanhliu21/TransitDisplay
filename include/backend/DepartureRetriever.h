#ifndef DEPARTURE_RETRIEVER_H
#define DEPARTURE_RETRIEVER_H

#include "types/DepartureList.h"
#include "types/RouteList.h"
#include "types/TransitTypes.h"
#include "backend/BaseRetriever.h"
#include "backend/APICaller.h"
#include "backend/TimeRetriever.h"

/**
 * Fetches departures from a SINGLE stop
 */
class DepartureRetriever : public BaseRetriever
{
public:
  DepartureRetriever(APICaller *caller,
                     TimeRetriever *time,
                     const Stop &stop,
                     const RouteList &routeList,
                     const int departureLimit,
                     const int nextNSeconds,
                     const int timestampCutoff);

  virtual bool retrieve() override;
  DepartureList getDepartureList() const;

protected:
  virtual void parseOneElement(JsonVariantConst &doc) override;

private:
  TimeRetriever *m_time;
  Stop m_stop;
  RouteList m_routeList;
  DepartureList m_departures;
  int m_departureLimit;
  int m_nextNSeconds;
  int m_timestampCutoff;

  static JsonDocument constructFilter();
  static std::string constructEndpointString(
      const Stop &stop, const int departureLimit, const int nextNSeconds);
  void parseOneDeparture(JsonVariantConst &doc);

  bool retrieveIsRealTime(JsonVariantConst &doc, Departure &dep);
  bool retrieveHeadsign(JsonVariantConst &doc, Departure &dep);
  bool retrieveRoute(JsonVariantConst &doc, Departure &dep);
  bool retrieveTimestampDelay(JsonVariantConst &doc, Departure &dep);

  std::time_t convertTime(const std::string &str);
};

#endif