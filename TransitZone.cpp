#include "TransitZone.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <StreamUtils.h>

#include <vector>

#include "RouteTable.h"
#include "StopTable.h"
#include "constants.h"
#include "arduino_secrets.h"

TransitZone::TransitZone(String name, RouteTable *routeTable, StopTable *stopTable, HTTPClient *client, const float lat, const float lon, const float radius)
  : m_name{name}, m_routeTable{routeTable}, m_stopTable{stopTable}, m_client{client}, m_lat{lat}, m_lon{lon}, m_radius{radius}, m_isValidZone{false}, m_whiteList{nullptr}
  {}

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
}

void TransitZone::setWhiteList(std::vector<String> *whiteList) {
  m_whiteList = whiteList;
}

void TransitZone::clearWhiteList() {
  m_whiteList = nullptr;
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
    // Serial.print("endpoint: ");
    // Serial.println(endpoint);
    m_client->collectHeaders(keys, 1);

    m_client->begin(TRANSIT_LAND_SERVER, TRANSIT_LAND_PORT, endpoint, TRANSIT_LAND_ROOT_CERTIFICATE);
    m_client->GET();

    // Get the raw and the decoded stream
    Stream& rawStream = m_client->getStream();
    ChunkDecodingStream decodedStream(rawStream);
    // Choose the right stream depending on the Transfer-Encoding header
    Stream& response =
        m_client->header("Transfer-Encoding") == "chunked" ? decodedStream : rawStream;
    
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
      m_client->end();
      return false;
    }
    if (responseDoc["meta"].isNull()
        || !responseDoc["meta"]["next"].is<const char*>()) {
      endpoint = "";
    } else {
      endpoint = responseDoc["meta"]["next"].as<String>();
    }

    // extract first stop
    if (responseDoc["stops"].isNull()) {
      Serial.println("stops key is not there");
      m_client->end();
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

      Stop stop(onestopId, name, feedId, NUM_ROUTES_STORED, m_routeTable, m_client);
      m_stops.push_back(m_stopTable->addStop(stop));

      stopCnt++;
    }
    
    m_client->end();
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