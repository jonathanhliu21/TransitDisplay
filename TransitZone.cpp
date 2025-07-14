#include "TransitZone.h"

#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>

#include "RouteTable.h"
#include "StopTable.h"
#include "constants.h"
#include "arduino_secrets.h"

TransitZone::TransitZone(String name, RouteTable *routeTable, StopTable *stopTable, HttpClient *client, const float lat, const float lon, const float radius)
  : m_name{name}, m_routeTable{routeTable}, m_stopTable{stopTable}, m_client{client}, m_lat{lat}, m_lon{lon}, m_radius{radius}
  {}

void TransitZone::init() {
  m_routes = m_routeTable->retrieveRoutes(m_lat, m_lon, m_radius);
  bool stopsValid = retrieveStops();

  m_isValidZone = m_routes.size() > 0 && stopsValid;
}

void TransitZone::debugPrint() const {
  Serial.print("---------- ");
  Serial.print(m_name);
  Serial.println(" ----------");

  Serial.println(m_isValidZone ? "Valid Zone" : "Invalid Zone");

  m_routeTable->debugPrintAllRoutes();
  m_stopTable->debugPrintAllStops();
}

bool TransitZone::retrieveStops() {
  String endpoint = String(STOPS_ENDPOINT_PREFIX) + "?api_key=" + SECRET_API_KEY + "&lat=" + String(m_lat, 6) + "&lon=" + String(m_lon, 6) + "&radius=" + m_radius;

  int loopCnt = 0;
  int stopCnt = 0;

   // for "next" pages
  while (endpoint != "" && loopCnt < 5) {
    // Serial.print("endpoint: ");
    // Serial.println(endpoint);

    m_client->get(endpoint);

    // Get the status code from the server's response
    int statusCode = m_client->responseStatusCode();
    if (statusCode != 200) {
      Serial.println("Status Code for stop did not return 200");
      return false;
    }

    // Create filter
    JsonDocument filter;
    filter["meta"]["next"] = true;
    JsonObject filter_stops_0 = filter["stops"].add<JsonObject>();
    filter_stops_0["feed_version"]["feed"]["onestop_id"] = true;
    filter_stops_0["stop_name"] = true;
    filter_stops_0["onestop_id"] = true;
    filter_stops_0["parent"]["stop_id"] = true;

    // Store response
    JsonDocument responseDoc;
    // Serial.println(m_client->responseBody());
    DeserializationError error = deserializeJson(responseDoc, m_client->responseBody(), DeserializationOption::Filter(filter));
    if (error) {
      Serial.println("Deserialization for stop failed");
      Serial.println(error.c_str());
      return false;
    }
    if (!responseDoc["meta"].is<JsonVariantConst>()
        || !responseDoc["meta"]["next"].is<const char*>()) {
      endpoint = "";
    } else {
      endpoint = responseDoc["meta"]["next"].as<String>();
    }

    // extract first stop
    if (!responseDoc["stops"].is<JsonArrayConst>()) {
      Serial.println("stops key is not there");
      return false;
    }
    JsonArrayConst arr = responseDoc["stops"].as<JsonArrayConst>();
    int size = arr.size();

    // Serial.print("size: ");
    // Serial.println(size);

    for (int i = 0; i < size; i++) {
      if (!arr[i].is<JsonVariantConst>()) {
        Serial.println("Not variant const");
        // Serial.print("Index ");
        // Serial.print(i);
        // Serial.println(" continued because not variant const");
        continue;
      }
      // Serial.println("Crash 4");
      JsonVariantConst stopInfo = arr[i].as<JsonVariantConst>();

      if (!stopInfo["parent"].isNull()) {
        // Serial.println("Parent is not null");
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
      if (!stopInfo["feed_version"].is<JsonVariantConst>()
          || !stopInfo["feed_version"]["feed"].is<JsonVariantConst>()
          || !stopInfo["feed_version"]["feed"]["onestop_id"].is<const char *>()) {
        Serial.println("Could not retrieve feed ID from stop");
        continue;
      }
      String feedId = stopInfo["feed_version"]["feed"]["onestop_id"].as<String>();

      Stop stop(onestopId, name, feedId, NUM_ROUTES_STORED, m_routeTable, m_client);
      m_stops.push_back(m_stopTable->addStop(stop));

      loopCnt++;
    }
  }

  return loopCnt > 0;
}