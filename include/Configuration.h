#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <vector>
#include <string>

#include "types/Whitelist.h"
#include "backend/TransitZone.h"
#include "backend/TimeRetriever.h"
#include "backend/APICaller.h"

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

  const std::vector<TransitZone *> getZones() const;
  const TimeRetriever *getTimeRetriever() const;
  const APICaller *getCaller() const;

  const Whitelist getWhitelist() const;
  const std::string getSSID() const;
  const std::string getWifiPassword() const;
  const std::string getAPIKey() const;

private:
  APICaller *m_caller;
  TimeRetriever m_retriever;

  std::vector<TransitZone *> m_zones;
  Whitelist m_whitelist;

  std::string m_ssid, m_password, m_apiKey;
};

#endif