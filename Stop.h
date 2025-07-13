#ifndef STOP_H
#define STOP_H

#include <Arduino.h>
#include "RouteTable.h"

#include <vector>
#include <ArduinoHttpClient.h>

struct Departure {
  Route *route;
  String direction;
  signed long long timestamp;
  bool isRealTime;
  String agency_id;
};

class Stop {
public:
  Stop(const String &oneStopId, const int &numDepartures, RouteTable *routeTable, HttpClient *client);

  void init();
  void callDeparturesAPI();

  bool getIsValidStop() const;
  String getId() const;
  int getNumDepartures() const;
  String getName() const;
  String getAgencyId() const;
  const std::vector<Departure> *getDepartures() const;
  float getLat() const;
  float getLon() const;

private:
  String m_id;
  int m_numDepartures;
  RouteTable *m_routeTable;
  HttpClient *m_client;

  String m_name;
  String m_agencyId;
  bool m_isValidStop;

  std::vector<Departure> m_departures;
  unsigned long long m_lastRetrieveTime;

  bool retrieveStopInfo();
  String truncateName(const String &name, const bool truncateDowntown) const;
};

#endif