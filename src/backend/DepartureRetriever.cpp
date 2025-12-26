#include "backend/BaseRetriever.h"
#include "backend/DepartureRetriever.h"

#include <time.h>
#include <string>
#include <ArduinoJson.h>

#include "Constants.h"
#include "types/TransitTypes.h"

namespace
{
  const int DEPARTURES_MAX_PAGES_PROCESSED = Constants::MAX_PAGES_PROCESSED;
  const char *DEPARTURES_STOPS_KEY_NAME = "stops";
  const char *DEPARTURES_KEY_NAME = "departures";
  const int DEPARTURES_NESTING_LIMIT = 20;
}

DepartureRetriever::DepartureRetriever(APICaller *caller,
                                       TimeRetriever *time,
                                       const Stop &stop,
                                       const RouteList &routeList,
                                       const int departureLimit,
                                       const int nextNSeconds,
                                       const int timestampCutoff)
    : BaseRetriever{
          caller,
          constructEndpointString(stop, departureLimit, nextNSeconds),
          DEPARTURES_MAX_PAGES_PROCESSED, Constants::DEPARTURE_ERROR_PIN},
      m_time{time}, m_stop{stop}, m_routeList{routeList}, m_departureLimit{departureLimit}, m_nextNSeconds{nextNSeconds}, m_timestampCutoff{timestampCutoff}
{
}

bool DepartureRetriever::retrieve()
{
  m_departures.clear();
  JsonDocument filter = constructFilter();
  return loopRequest(filter, DEPARTURES_STOPS_KEY_NAME, DEPARTURES_NESTING_LIMIT);
}

DepartureList DepartureRetriever::getDepartureList() const
{
  return m_departures;
}

void DepartureRetriever::parseOneElement(JsonVariantConst &stopInfo)
{
  // check that the location type is an actual stop and not some random exit
  if (stopInfo["location_type"].isNull() || stopInfo["location_type"].as<int>() != 0)
  {
    return;
  }

  // extract info from stop
  // departures
  if (!stopInfo["departures"].is<JsonArrayConst>() || stopInfo["departures"].size() <= 0)
  {
    return;
  }

  // each stop has a departures array
  // extract the departures from each stop
  JsonArrayConst departuresArr = stopInfo["departures"].as<JsonArrayConst>();
  int departuresArrSize = departuresArr.size();
  for (int j = 0; j < departuresArrSize; j++)
  {
    // make sure the departure exists
    if (departuresArr[j].isNull() || !departuresArr[j].is<JsonObjectConst>())
      continue;
    JsonVariantConst departureDoc = departuresArr[j];
    if (departureDoc["trip"].isNull())
      continue;

    parseOneDeparture(departureDoc);
  }
}

JsonDocument DepartureRetriever::constructFilter() const
{
  JsonDocument filter;
  filter["meta"]["next"] = true;

  JsonObject filter_stops_0 = filter["stops"].add<JsonObject>();
  filter_stops_0["location_type"] = true;

  JsonObject filter_stops_0_departures_0 = filter_stops_0["departures"].add<JsonObject>();
  filter_stops_0_departures_0["schedule_relationship"] = true;
  filter_stops_0_departures_0["stop_headsign"] = true;

  JsonObject filter_stops_0_departures_0_departure = filter_stops_0_departures_0["departure"].to<JsonObject>();
  filter_stops_0_departures_0_departure["estimated_utc"] = true;
  filter_stops_0_departures_0_departure["estimated_delay"] = true;
  filter_stops_0_departures_0_departure["scheduled_utc"] = true;

  JsonObject filter_stops_0_departures_0_trip = filter_stops_0_departures_0["trip"].to<JsonObject>();
  filter_stops_0_departures_0_trip["schedule_relationship"] = true;
  filter_stops_0_departures_0_trip["trip_headsign"] = true;
  filter_stops_0_departures_0_trip["route"]["onestop_id"] = true;
  filter_stops_0_departures_0_trip["route"]["agency"]["onestop_id"] = true;

  return filter;
}

std::string DepartureRetriever::constructEndpointString(
    const Stop &stop, const int departureLimit, const int nextNSeconds)
{
  std::string res = Constants::STOPS_ENDPOINT_PREFIX;
  res += std::string("/") + stop.onestopId + "/departures";
  res += std::string("?limit=") + std::to_string(departureLimit);
  res += std::string("&next=") + std::to_string(nextNSeconds);
  res += "&include_alerts=true&use_service_window=false";
  return res;
}

void DepartureRetriever::parseOneDeparture(JsonVariantConst &departureDoc)
{
  Departure departure;
  departure.stop = m_stop;
  departure.isValid = true;

  if (!retrieveIsRealTime(departureDoc, departure))
    return;
  if (!retrieveHeadsign(departureDoc, departure))
    return;
  if (!retrieveRoute(departureDoc, departure))
    return;
  if (!retrieveTimestampDelay(departureDoc, departure))
    return;

  m_departures.addDeparture(departure);
}

bool DepartureRetriever::retrieveIsRealTime(JsonVariantConst &departureDoc, Departure &departure)
{
  departure.isRealTime = true;
  if (departureDoc["schedule_relationship"].isNull() && (departureDoc["trip"]["schedule_relationship"].isNull()))
  {
    departure.isRealTime = false;
  }
  else
  {
    // schedule relationship
    std::string schedule_relationship;
    if (departureDoc["schedule_relationship"].isNull())
    {
      schedule_relationship = departureDoc["trip"]["schedule_relationship"].as<std::string>();
    }
    else
    {
      schedule_relationship = departureDoc["schedule_relationship"].as<std::string>();
    }
    if (schedule_relationship == "STATIC" || schedule_relationship == "NO_DATA")
    {
      departure.isRealTime = false;
    }

    if (schedule_relationship == "DELETED" || schedule_relationship == "SKIPPED" || schedule_relationship == "CANCELED")
      return false;
  }

  return true;
}

bool DepartureRetriever::retrieveHeadsign(JsonVariantConst &departureDoc, Departure &departure)
{
  std::string headsign;
  // look for key "stop_headsign" or ["trip"]["trip_headsign"]
  // prioritize ["trip"]["trip_headsign"]
  if (departureDoc["trip"]["trip_headsign"].isNull())
  {
    if (departureDoc["stop_headsign"].isNull())
      return false;
    headsign = departureDoc["stop_headsign"].as<std::string>();
  }
  else
  {
    headsign = departureDoc["trip"]["trip_headsign"].as<std::string>();
  }
  departure.direction = headsign;

  return true;
}

bool DepartureRetriever::retrieveRoute(JsonVariantConst &departureDoc, Departure &departure)
{
  // make sure that the route exists
  if (departureDoc["trip"]["route"].isNull() ||
      departureDoc["trip"]["route"]["onestop_id"].isNull())
    return false;

  // get route of onestop ID, and get route object from onestop ID
  std::string onestopId = departureDoc["trip"]["route"]["onestop_id"].as<std::string>();
  if (!m_routeList.routeExists(onestopId))
    return false;
  departure.route = m_routeList.getRoute(onestopId);
  if (departureDoc["trip"]["route"]["agency"].isNull() || departureDoc["trip"]["route"]["agency"]["onestop_id"].isNull())
    return false;
  departure.agencyOnestopId = departureDoc["trip"]["route"]["agency"]["onestop_id"].as<std::string>();

  return true;
}

bool DepartureRetriever::retrieveTimestampDelay(JsonVariantConst &departureDoc, Departure &departure)
{
  if (departureDoc["departure"].isNull())
    return false;
  JsonVariantConst depInfo = departureDoc["departure"];

  std::string timestampActualStr;
  std::string timestampExpectedStr;
  bool delayKeyFound = false;

  // set to scheduled UTC, if null it will be empty string
  if (!depInfo["scheduled_utc"].isNull())
    timestampExpectedStr = depInfo["scheduled_utc"].as<std::string>(); // aka scheduled

  // default value - will be set later
  departure.delay = 0;

  if (!departure.isRealTime || depInfo["estimated_utc"].isNull())
  {
    // if not real time, set actual time to scheduled time
    departure.isRealTime = false;
    if (depInfo["scheduled_utc"].isNull())
    {
      return false;
    }
    timestampActualStr = timestampExpectedStr;
  }
  else
  {
    // treat as if real time
    // estimated_utc is guaranteed not to be null
    timestampActualStr = depInfo["estimated_utc"].as<std::string>();

    // set delay if found in response
    if (!depInfo["estimated_delay"].isNull())
    {
      departure.delay = depInfo["estimated_delay"].as<int>();
      delayKeyFound = true;
    }
  }

  // convert times
  departure.actualTimestamp = convertTime(timestampActualStr);
  departure.expectedTimestamp = convertTime(timestampExpectedStr);

  // manually calculate delay if not found in response
  // this means that timestamp expected string
  if (!delayKeyFound && timestampExpectedStr != "" && departure.isRealTime)
  {
    departure.delay = static_cast<long long>(departure.actualTimestamp) - static_cast<long long>(departure.expectedTimestamp);
  }

  time_t curTime = m_time->getCurTime();
  if (departure.actualTimestamp < curTime - m_timestampCutoff)
  {
    return false;
  }

  return true;
}

time_t DepartureRetriever::convertTime(const std::string &str)
{
  struct tm timeinfo;
  strptime(str.c_str(), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return m_time->timegmUTC(&timeinfo);
}