#ifndef BRIDGE_H
#define BRIDGE_H

#include <Arduino.h>
#include <vector>
#include <mutex>

#include "TransitZone.h"
#include "RouteTable.h"

struct BridgeDeparture {
  String direction;
  int mins;
  String line;
  int textColor;
  int routeColor;
  int delayColor;
};

// Bridges the code between calling the API and updating the display
class Bridge {
public:
  Bridge();
  ~Bridge();
  void setZone(TransitZone *zone, time_t startTimeMillis, time_t startTimeUTC);
  void retrieveDepartures();

  void debugPrintRoutes() const;
  void debugPrintDepartures() const;

  void stop();

private:
  TransitZone *m_zone;
  String m_name;
  std::vector<Route> m_routes;
  std::vector<BridgeDeparture> m_departures;
  time_t m_startTimeS, m_startTimeUTC;

  // Mutex to protect access to the m_departures vector.
  std::mutex* m_departures_mutex;
  // Handle for the FreeRTOS task.
  TaskHandle_t m_retrieval_thread_handle = NULL;

  time_t retrieveTime() const;
  int getDelayColor(int delay, bool isRealTime) const;

  void modifyRoutes();
  Departure modifyDeparture(const Departure &dep);
  String truncateStop(const String &stopName, const bool truncateDowntown) const;
  String truncateRoute(const String &routeName) const;
  void sortRoutes(std::vector<Route *> &routes) const;

  static void retrieval_task_runner(void *pvParameters);
  void retrieval_loop();
};

#endif