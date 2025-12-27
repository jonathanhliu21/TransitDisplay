#include "backend/BaseRetriever.h"
#include "backend/StopRetriever.h"

#include <string>
#include <ArduinoJson.h>
#include <Arduino.h>

#include "Constants.h"
#include "types/TransitTypes.h"
#include "types/Whitelist.h"

namespace
{
  const int STOP_MAX_PAGES_PROCESSED = Constants::MAX_PAGES_PROCESSED;
  const char *STOP_KEY_NAME = "stops";
}

StopRetriever::StopRetriever(APICaller *caller, float lat, float lon, float radius)
    : BaseRetriever{
          caller,
          constructEndpointString(lat, lon, radius),
          STOP_MAX_PAGES_PROCESSED,
          Constants::STOP_ERROR_PIN},
      m_lat{lat}, m_lon{lon}, m_radius{radius}
{
}

StopRetriever::StopRetriever(APICaller *caller, float lat, float lon, float radius, const Whitelist &whitelist)
    : BaseRetriever{
          caller,
          constructEndpointString(lat, lon, radius, whitelist),
          STOP_MAX_PAGES_PROCESSED,
          Constants::STOP_ERROR_PIN},
      m_lat{lat}, m_lon{lon}, m_radius{radius}, m_whitelist{whitelist}
{
}

bool StopRetriever::retrieve()
{
  m_stopList.clear();
  JsonDocument filter = constructFilter();
  return loopRequest(filter, STOP_KEY_NAME);
}

StopList StopRetriever::getStopList() const
{
  return m_stopList;
}

Whitelist StopRetriever::getWhiteList() const
{
  return m_whitelist;
}

void StopRetriever::setWhiteList(const Whitelist &whitelist)
{
  m_whitelist = whitelist;
  setEndpoint(constructEndpointString(m_lat, m_lon, m_radius, m_whitelist));
}

void StopRetriever::parseOneElement(JsonVariantConst &stopInfo)
{
  // check that the location type is an actual stop and not some random exit
  if (stopInfo["location_type"].isNull() || stopInfo["location_type"].as<int>() != 0)
  {
    return;
  }

  // get info from stop
  Stop stop;
  stop.onestopId = stopInfo["onestop_id"].as<std::string>();
  stop.name = stopInfo["stop_name"].as<std::string>();

  m_stopList.addStop(stop);
}

JsonDocument StopRetriever::constructFilter()
{
  // Create filter
  JsonDocument filter;
  filter["meta"]["next"] = true;
  JsonObject filter_stops_0 = filter["stops"].add<JsonObject>();
  filter_stops_0["stop_name"] = true;
  filter_stops_0["onestop_id"] = true;
  filter_stops_0["location_type"] = true;

  return filter;
}

std::string StopRetriever::constructEndpointString(float lat, float lon, float radius)
{
  std::string res = Constants::STOPS_ENDPOINT_PREFIX;
  res += "?lat=" + std::to_string(lat);
  res += "&lon=" + std::to_string(lon);
  res += "&radius=" + std::to_string(radius);
  return res;
}

std::string StopRetriever::constructEndpointString(
    float lat, float lon, float radius, const Whitelist &whitelist)
{
  std::string res = Constants::STOPS_ENDPOINT_PREFIX;
  res += "?lat=" + std::to_string(lat);
  res += "&lon=" + std::to_string(lon);
  res += "&radius=" + std::to_string(radius);
  if (whitelist.isActive())
  {
    res += "&served_by_onestop_ids=" + whitelist.getWhiteListStr();
  }
  return res;
}