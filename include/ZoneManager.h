#ifndef ZONE_MANAGER_H
#define ZONE_MANAGER_H

#include <TFT_eSPI.h>
#include <vector>
#include <mutex>
#include <ctime>
#include <string>

#include "backend/TimeRetriever.h"
#include "backend/TransitZone.h"
#include "types/TransitTypes.h"
#include "types/DisplayTypes.h"
#include "types/RouteList.h"
#include "types/DepartureList.h"
#include "types/Whitelist.h"
#include "frontend/TransitZoneDisplayer.h"

class ZoneManager
{
public:
  ZoneManager(TransitZone *zone,
              TFT_eSPI *tft,
              TimeRetriever *timeRetriever,
              const Whitelist &whitelist,
              const uint8_t *fontRegular,
              const uint8_t *fontLarge);
  ~ZoneManager();

  void init();
  void mainThreadLoop();
  void stop();
  void drawAreYouSure();

private:
  TransitZone *m_zone;
  TimeRetriever *m_timeRetriever;
  Whitelist m_whitelist;
  TransitZoneDisplayer m_displayer; // shared variable!

  std::mutex m_displayerMtx;
  TaskHandle_t m_retrieval_thread_handle = NULL;

  static void retrievalTaskRunner(void *pvParameters);
  void bgTaskLoop();
  void safeSetDisplayDeps(const std::vector<DisplayDeparture> &deps);
};

#endif