#include "Configuration.h"

#include "secrets.h"
#include "UserConfig.h"
#include "backend/DepartureRetriever.h"

namespace
{
  // store 7 departures at a time
  // retrieve departures for next 100 mins (6000 s)
  // cut off departures more than 60s ago
  const DepartureRetrieverConfig DEFAULT_TRANSIT_ZONE_CONFIG = {
      7, 6000, 60};
}

Configuration::~Configuration()
{
  delete m_caller;
  for (int i = 0; i < m_zones.size(); i++)
  {
    delete m_zones[i];
  }
}

void Configuration::init()
{
  // subject to change, right now all configs are hardcoded

  // secrets
  m_ssid = Secrets::SECRET_SSID;
  m_password = Secrets::SECRET_PASSWORD;
  m_apiKey = Secrets::SECRET_API_KEY;

  // API caller
  m_caller = new APICaller(m_apiKey);

  // whitelist
  m_whitelist.setActive(userWhiteListActive);
  for (auto str : userWhiteList)
  {
    m_whitelist.addItem(str);
  }

  // transitzone
  for (const UserTransitZone &zone : userTransitZoneList)
  {
    TransitZone *z = new TransitZone(zone.name,
                                     zone.lat,
                                     zone.lon,
                                     zone.radius,
                                     m_caller,
                                     &m_retriever,
                                     DEFAULT_TRANSIT_ZONE_CONFIG);
    m_zones.push_back(z);
  }
}

const std::vector<TransitZone *> Configuration::getZones() const { return m_zones; }
const TimeRetriever *Configuration::getTimeRetriever() const { return &m_retriever; }
const APICaller *Configuration::getCaller() const { return m_caller; }

const Whitelist Configuration::getWhitelist() const { return m_whitelist; }
const std::string Configuration::getSSID() const { return m_ssid; }
const std::string Configuration::getWifiPassword() const { return m_password; }
const std::string Configuration::getAPIKey() const { return m_apiKey; }