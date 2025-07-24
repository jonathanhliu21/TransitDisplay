#include "TransitZone.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <StreamUtils.h>

#include <algorithm>
#include <ctime>
#include <cstring>
#include <time.h>
#include <vector>

#include "RouteTable.h"
#include "StopTable.h"
#include "constants.h"
#include "arduino_secrets.h"

void debugPrintDepartures(const std::vector<Departure> &departures) {
  // Check if there are any departures to print
  if (departures.empty()) {
    Serial.println(F("    No departures found."));
    return;
  }

  // Iterate over each departure in the vector
  for (const auto& departure : departures) {
    Serial.println(F("    ----------------------"));

    // Print Route ID, ensuring the route pointer is not null
    if (departure.route) {
      Serial.print(F("    Route: "));
      Serial.println(departure.route->id);
    } else {
      Serial.println(F("    Route: [Unknown]"));
    }

    // Print Direction
    Serial.print(F("    Direction: "));
    Serial.println(departure.direction);

    // Print Timestamp
    Serial.print(F("    Timestamp: "));
    Serial.println(departure.timestamp);

    // Print Real-Time Status
    Serial.print(F("    Is Real-Time: "));
    Serial.println(departure.isRealTime ? "Yes" : "No");

    // Print Agency ID
    Serial.print(F("    Agency: "));
    Serial.println(departure.agency_id);

    // Print Delay
    Serial.print(F("    Delay (seconds): "));
    Serial.println(departure.delay);

    // Print is valid
    Serial.print(F("    Is Valid: "));
    Serial.println(departure.isValid ? "Yes" : "No");
  }
  Serial.println(F("    ----------------------"));
}


TransitZone::TransitZone(String name, RouteTable *routeTable, StopTable *stopTable, const float lat, const float lon, const float radius)
  : m_name{name}, m_routeTable{routeTable}, m_stopTable{stopTable}, m_lat{lat}, m_lon{lon}, m_radius{radius}, m_isValidZone{false}, m_whiteList{nullptr}
  {}

String TransitZone::getName() const { return m_name; }
bool TransitZone::getIsValidZone() const { return m_isValidZone; }
std::vector<Route *> TransitZone::getRoutes() const { return m_routes; }
std::vector<Departure> TransitZone::getDepartures() const { return m_departures; }

void TransitZone::init() {
  m_routes = m_routeTable->retrieveRoutes(m_lat, m_lon, m_radius, m_whiteList);
  bool stopsValid = retrieveStops();

  m_isValidZone = m_routes.size() > 0 && stopsValid;
}

void TransitZone::debugPrint() const {
  Serial.print("---------- ");
  Serial.print(m_name);
  Serial.println(" ----------");

  Serial.println(m_isValidZone ? "Valid Zone" : "Invalid Zone");

  m_routeTable->debugPrintAllRoutes();
  // m_stopTable->debugPrintAllStops();

  debugPrintDepartures(m_departures);
}

void TransitZone::setWhiteList(std::vector<String> *whiteList) {
  m_whiteList = whiteList;
}

void TransitZone::clearWhiteList() {
  m_whiteList = nullptr;
}

void TransitZone::updateDepartures(std::time_t curTime) {
  checkDepTimes(m_departures, curTime);

  // clear departures before current time
  while (m_departures.size() > 0) {
    // Serial.print("departures size: ");
    // Serial.println(m_departures.size());
    if (curTime > m_departures.begin()->timestamp) {
      m_departures.erase(m_departures.begin());
    } else {
      break;
    }
  }

  m_departures.clear();

  for (Stop *stop : m_stops) {
    bool res = stop->callDeparturesAPI(curTime);
    // Serial.print("Retrieving result: ");
    // Serial.println(res);

    if (!res) continue;
    std::vector<Departure> deps = stop->getDepartures();
    checkDepTimes(deps, curTime);
    // Serial.println(deps.size());
    // debugPrintDepartures(deps);
    combineDepartures(m_departures, deps);
  }

  checkDepTimes(m_departures, curTime);
}

bool TransitZone::retrieveStops() {
  String endpoint = String(STOPS_ENDPOINT_PREFIX) + "?api_key=" + SECRET_API_KEY + "&lat=" + String(m_lat, 6) + "&lon=" + String(m_lon, 6) + "&radius=" + m_radius;

  if (m_whiteList != nullptr) {
    endpoint += "&served_by_onestop_ids=" + getWhiteListIds();
  }

  int loopCnt = 0;
  int stopCnt = 0;

  const char* keys[] = {"Transfer-Encoding"};

   // for "next" pages
  while (endpoint != "" && loopCnt < MAX_PAGES_PROCESSED) {
    HTTPClient client;
    // Serial.print("endpoint: ");
    // Serial.println(endpoint);
    
    client.collectHeaders(keys, 1);
    client.setTimeout(HTTP_TIMEOUT);

    client.begin(TRANSIT_LAND_SERVER, TRANSIT_LAND_PORT, endpoint, TRANSIT_LAND_ROOT_CERTIFICATE);
    int httpCode = client.GET();

    if (httpCode != 200) {
      Serial.print("Stop Http request failed with code: ");
      Serial.println(httpCode);
      client.end();
      return false;
    }

    // Get the raw and the decoded stream
    Stream& rawStream = client.getStream();
    ChunkDecodingStream decodedStream(rawStream);
    // Choose the right stream depending on the Transfer-Encoding header
    Stream& response =
        client.header("Transfer-Encoding") == "chunked" ? decodedStream : rawStream;
    
    // Serial.print("is chunked: ");
    // Serial.println(m_client->header("Transfer-Encoding") == "chunked");

    // Create filter
    JsonDocument filter;
    filter["meta"]["next"] = true;
    JsonObject filter_stops_0 = filter["stops"].add<JsonObject>();
    filter_stops_0["feed_version"]["feed"]["onestop_id"] = true;
    filter_stops_0["stop_name"] = true;
    filter_stops_0["onestop_id"] = true;
    filter_stops_0["location_type"] = true;

    // Store response
    JsonDocument responseDoc;
    // Serial.println(m_client->responseBody());
    DeserializationError error = deserializeJson(responseDoc, response, DeserializationOption::Filter(filter));
    if (error) {
      Serial.println("Deserialization for stop failed");
      Serial.println(error.c_str());
      client.end();
      return false;
    }
    if (responseDoc["meta"].isNull()
        || !responseDoc["meta"]["next"].is<const char*>()) {
      endpoint = "";
    } else {
      endpoint = responseDoc["meta"]["next"].as<String>();
      endpoint = endpoint.substring(strlen(TRANSIT_LAND_HTTPS));
    }

    // extract first stop
    if (responseDoc["stops"].isNull()) {
      Serial.println("stops key is not there");
      client.end();
      return false;
    }
    JsonArrayConst arr = responseDoc["stops"].as<JsonArrayConst>();
    int size = arr.size();

    // Serial.print("size: ");
    // Serial.println(size);

    for (int i = 0; i < size; i++) {
      if (arr[i].isNull()) {
        Serial.println("Not variant const");
        // Serial.print("Index ");
        // Serial.print(i);
        // Serial.println(" continued because not variant const");
        continue;
      }
      // Serial.println("Crash 4");
      JsonVariantConst stopInfo = arr[i].as<JsonVariantConst>();

      // check that the location type is an actual stop and not some random exit
      if (stopInfo["location_type"].isNull() || stopInfo["location_type"].as<int>() != 0) {
        continue;
      }

      // extract info from stop
      // id
      if (!stopInfo["onestop_id"].is<const char *>()) {
        Serial.println("Stop name doesn't exist or is wrong type");
        continue;
      }
      String onestopId = stopInfo["onestop_id"].as<String>();
      // stop name
      if (!stopInfo["stop_name"].is<const char *>()) {
        Serial.println("Stop name doesn't exist or is wrong type");
        continue;
      }
      String name = stopInfo["stop_name"].as<String>();
      // then feed id
      if (stopInfo["feed_version"].isNull()
          || stopInfo["feed_version"]["feed"].isNull()
          || !stopInfo["feed_version"]["feed"]["onestop_id"].is<const char *>()) {
        Serial.println("Could not retrieve feed ID from stop");
        continue;
      }
      String feedId = stopInfo["feed_version"]["feed"]["onestop_id"].as<String>();

      Stop stop(onestopId, name, feedId, NUM_ROUTES_STORED, m_routeTable);
      m_stops.push_back(m_stopTable->addStop(stop));

      stopCnt++;
    }
    
    client.end();
    loopCnt++;
  }

  return stopCnt > 0;
}

String TransitZone::getWhiteListIds() const {
  String res;
  if (m_whiteList == nullptr) return res;

  for (int i = 0; i < m_whiteList->size(); i++) {
    res += m_whiteList->at(i) + ",";
  }

  return res;
}

void TransitZone::combineDepartures(std::vector<Departure> &a, const std::vector<Departure> &b) {
  std::vector<Departure> vec;
  for (Departure dep : a) {
    if (dep.isValid) {
      vec.push_back(dep);
    }
  }

  for (Departure dep : b) {
    if (dep.isValid) {
      vec.push_back(dep);
    }
  }

  std::sort(vec.begin(), vec.end(), [](const Departure &a, const Departure &b) {
    return a.timestamp < b.timestamp;
  });

  // Serial.println(b.size());
  // Serial.println(vec.size());

  a.clear();
  for (int i = 0; i < std::min((int) vec.size(), NUM_ROUTES_STORED); i++) {
    a.push_back(vec[i]);
  }
}

void TransitZone::checkDepTimes(std::vector<Departure> &deps, std::time_t curTime) {
  // update departure if it's a week ahead
  for (int i = 0; i < deps.size(); i++) {
    if (deps[i].agency_id == METRO_LOS_ANGELES && (long long) deps[i].timestamp - (long long) curTime > FIVE_DAYS) {
      deps[i].timestamp -= SEVEN_DAYS;
    }
  }
}