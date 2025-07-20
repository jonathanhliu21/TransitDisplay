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
  int delay;
  bool isValid;
};

class Stop {
public:
  Stop(const String &oneStopId, const String &name, const String &feedId, const int &numDepartures, RouteTable *routeTable, HTTPClient *client);

  bool callDeparturesAPI();

  String getId() const;
  int getNumDepartures() const;
  String getName() const;
  String getFeedId() const;
  std::vector<Departure> getDepartures() const;
  float getLat() const;
  float getLon() const;
  void debugPrintStop() const;

private:
  String m_id;
  int m_numDepartures;
  RouteTable *m_routeTable;
  HTTPClient *m_client;

  String m_name;
  String m_feedId;

  std::vector<Departure> m_departures;
  unsigned long long m_lastRetrieveTime;

  String truncateName(const String &name, const bool truncateDowntown) const;
};

#endif