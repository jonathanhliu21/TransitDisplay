#ifndef TRANSIT_ZONE_DISPLAYER_H
#define TRANSIT_ZONE_DISPLAYER_H

#include <ctime>
#include <string>
#include <vector>
#include <TFT_eSPI.h>

#include "types/DisplayTypes.h"
#include "frontend/BaseDisplayer.h"
#include "frontend/RouteDisplayer.h"
#include "frontend/DeparturesDisplayer.h"

class TransitZoneDisplayer : public BaseDisplayer
{
public:
  TransitZoneDisplayer(const std::string &name,
                       TFT_eSPI *tft,
                       const uint8_t *fontRegular,
                       const uint8_t *fontLarge,
                       int routeRefreshPeriodMs,
                       int departuresRefreshPeriodMs);

  void setRoutes(const std::vector<DisplayRoute> &displayRoutes);
  void setDepartures(const std::vector<DisplayDeparture> &displayDepartures);
  void drawInitializing();
  void drawAreYouSure();

  void cycle();
  void loop();

private:
  std::string m_name;
  TFT_eSPI *m_tft;
  const uint8_t *m_fontRegular;
  const uint8_t *m_fontLarge;
  int m_routeRefreshPeriod, m_departuresRefreshPeriod;
  std::time_t m_lastRouteRefresh, m_lastDeparturesRefresh;

  RouteDisplayer m_routeDisplay;
  DeparturesDisplayer m_departuresDisplay;

  void drawTitle();
};

#endif