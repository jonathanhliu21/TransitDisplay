#ifndef STOP_H
#define STOP_H

#include "Arduino.h"
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
  const std::vector<Route *> *getRoutes() const;
  float getLat() const;
  float getLon() const;

private:
  String m_id;
  int m_numDepartures;
  RouteTable *m_routeTable;
  HttpClient *m_client;

  String m_name;
  String m_agencyId;
  float m_lat, m_lon;
  bool m_isValidStop;

  std::vector<Departure> m_departures;
  std::vector<Route *> m_routes;
  unsigned long long m_lastRetrieveTime;

  bool retrieveStopInfo();
  bool retrieveRoutesInfo();
  String truncateRoute(const String &line);
  String truncateName(const String &name, const bool truncateDowntown);
};

#endif