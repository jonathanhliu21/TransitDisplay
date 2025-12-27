#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Arduino.h>
#include <vector>
#include <string>
#include <TFT_eSPI.h>

#include "types/Whitelist.h"
#include "backend/TransitZone.h"
#include "backend/TimeRetriever.h"
#include "backend/APICaller.h"
#include "frontend/ZoneListDisplayer.h"

struct UserTransitZone
{
  std::string name;
  float lat, lon, radius;
};

using UserTransitZoneList = std::vector<UserTransitZone>;

class Configuration
{
public:
  Configuration() = default;
  ~Configuration();

  void init();

  std::vector<TransitZone *> getZones() const;
  TimeRetriever *getTimeRetriever();
  APICaller *getCaller() const;
  TFT_eSPI *getTFT();
  ZoneListDisplayer *getZoneListDisplayer();

  const Whitelist getWhitelist() const;
  const std::string getSSID() const;
  const std::string getWifiPassword() const;
  const std::string getAPIKey() const;

  const uint8_t *getRegularFont() const;
  const uint8_t *getTitleFont() const;

private:
  TFT_eSPI m_tft;
  APICaller *m_caller;
  TimeRetriever m_timeRetriever;
  ZoneListDisplayer *m_zoneListDisplayer;

  std::vector<TransitZone *> m_zones;
  Whitelist m_whitelist;

  std::string m_ssid, m_password, m_apiKey;

  const uint8_t *m_regularFont;
  const uint8_t *m_titleFont;
};

#endif