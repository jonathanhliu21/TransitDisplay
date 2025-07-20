#ifndef STOP_H
#define STOP_H

#include <Arduino.h>
#include "RouteTable.h"

#include <vector>
#include <ctime>
#include <HTTPClient.h>

struct Departure {
  Route *route;
  String direction;
  std::time_t timestamp;
  bool isRealTime;
  String agency_id;
  long long delay;
  bool isValid;
};

class Stop {
public:
  Stop(const String &oneStopId, const String &name, const String &feedId, const int &numDepartures, RouteTable *routeTable);

  bool callDeparturesAPI();

  String getId() const;
  int getNumDepartures() const;
  String getName() const;
  String getFeedId() const;
  std::vector<Departure> getDepartures() const;
  float getLat() const;
  float getLon() const;
  void debugPrintStop() const;

  static time_t timegm_custom(struct tm *timeinfo);

private:
  String m_id;
  int m_numDepartures;
  RouteTable *m_routeTable;

  String m_name;
  String m_feedId;

  std::vector<Departure> m_departures;
  unsigned long long m_lastRetrieveTime;
};

#endif