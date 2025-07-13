#include "Stop.h"

#include <cstdlib>
#include <vector>
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>

#include "constants.h"
#include "RouteTable.h"
#include "arduino_secrets.h"

Stop::Stop(const String &oneStopId, const int &numDepartures, RouteTable *routeTable, HttpClient *client)
  : m_id{ oneStopId }, m_numDepartures{ numDepartures }, m_routeTable{ routeTable }, m_client{ client },
    m_lat{0}, m_lon{0}, m_isValidStop{ false }, m_lastRetrieveTime{ 0 } {
  m_departures.reserve(m_numDepartures);
}

// getters
bool Stop::getIsValidStop() const {
  return m_isValidStop;
}
String Stop::getId() const {
  return m_id;
}
int Stop::getNumDepartures() const {
  return m_numDepartures;
}
String Stop::getName() const {
  return m_name;
}
String Stop::getAgencyId() const {
  return m_agencyId;
}
const std::vector<Departure> *Stop::getDepartures() const {
  return &m_departures;
}
const std::vector<Route *> *Stop::getRoutes() const {
  return &m_routes;
}
float Stop::getLat() const {return m_lat;}
float Stop::getLon() const {return m_lon;}

void Stop::init() {
  // Test cases

  bool validStop = retrieveStopInfo();
  bool validRoutes = retrieveRoutesInfo();

  m_isValidStop = validStop && validRoutes;
}

bool Stop::retrieveStopInfo() {
  String endpoint = STOPS_ENDPOINT_PREFIX + m_id + "?api_key=" + SECRET_API_KEY;
  m_client->get(endpoint);

  // Get the status code from the server's response
  int statusCode = m_client->responseStatusCode();
  if (statusCode != 200) {
    Serial.println("Status Code for stop did not return 200");
    return false;
  }

  // Create filter
  JsonDocument filter;
  filter["stops"][0]["feed_version"]["feed"]["onestop_id"] = true;
  filter["stops"][0]["geometry"]["coordinates"] = true;
  filter["stops"][0]["stop_name"] = true;

  // Store response
  JsonDocument responseDoc;
  DeserializationError error = deserializeJson(responseDoc, m_client->responseBody(), DeserializationOption::Filter(filter));
  if (error) {
    Serial.println("Deserialization for routes failed");
    return false;
  }

  // extract first stop
  if (!responseDoc["stops"].is<JsonArray>()) {
    Serial.println("stops key is not there");
    return false;
  }
  JsonArray arr = responseDoc["stops"];
  if (arr.size() < 1) {
    Serial.println("arr size is 0");
    return false;
  }
  JsonVariant stopInfo = arr[0];

  // extract info from stop
  // first extract stop name
  if (!stopInfo["stop_name"].is<const char *>()) {
    Serial.println("Stop name doesn't exist or is wrong type");
    return false;
  }
  m_name = truncateName(stopInfo["stop_name"].as<String>(), false); // Don't truncate "Downtown" when retrieving station name
  // then agency id
  if (!stopInfo["feed_version"].is<JsonVariant>()
      || !stopInfo["feed_version"]["feed"].is<JsonVariant>()
      || !stopInfo["feed_version"]["feed"]["onestop_id"].is<const char *>()) {
    Serial.println("Could not retrieve agency ID from stop");
    return false;
  }
  m_agencyId = stopInfo["feed_version"]["feed"]["onestop_id"].as<String>();
  // then coords
  if (!stopInfo["geometry"].is<JsonVariant>()
      || !stopInfo["geometry"]["coordinates"].is<JsonArray>()
      || stopInfo["geometry"]["coordinates"].size() < 2
      || !stopInfo["geometry"]["coordinates"][0].is<float>()
      || !stopInfo["geometry"]["coordinates"][1].is<float>()) {
    Serial.println("Could not coordinates from stop");
    return false;
  }
  m_lon = stopInfo["geometry"]["coordinates"][0];
  m_lat = stopInfo["geometry"]["coordinates"][1];

  return true;
}

bool Stop::retrieveRoutesInfo() {
  String endpoint = String(ROUTES_ENDPOINT_PREFIX) + "?api_key=" + SECRET_API_KEY + "&lat=" + String(m_lat, 6) + "&lon=" + String(m_lon, 6) + "&radius=" + SEARCH_RADIUS;
  m_client->get(endpoint);

  // Get the status code from the server's response
  int statusCode = m_client->responseStatusCode();
  if (statusCode != 200) {
    Serial.println("Status Code for stop did not return 200");
    return false;
  }

  // Create filter
  JsonDocument filter;
  JsonObject filter_routes_0 = filter["routes"].add<JsonObject>();
  filter_routes_0["agency"]["onestop_id"] = true;
  filter_routes_0["onestop_id"] = true;
  filter_routes_0["route_color"] = true;
  filter_routes_0["route_text_color"] = true;
  filter_routes_0["route_short_name"] = true;
  filter_routes_0["route_long_name"] = true;

  // Store response
  JsonDocument responseDoc;
  DeserializationError error = deserializeJson(responseDoc, m_client->responseBody(), DeserializationOption::Filter(filter));
  if (error) {
    Serial.println("Deserialization for stop failed");
    return false;
  }
    // find if routes exist
  if (!responseDoc["routes"].is<JsonArray>()) {
    Serial.println("routes key is not there");
    return false;
  }

  Serial.println(responseDoc["routes"].as<JsonArray>().size());
  // for (JsonObject routeDoc : responseDoc["routes"].as<JsonArray>()) {
  //   // Serial.println("here!!!");
  // }

  
  // for (JsonObject routeDoc : responseDoc["routes"].as<JsonArray>()) {
    // Serial.println("here!!!");
    // if (!routeDoc["onestop_id"].is<const char*>()
    //     || !routeDoc["agency"].is<JsonVariant>()
    //     || !routeDoc["agency"]["onestop_id"].is<const char*>()
    //     || (!routeDoc["route_long_name"].is<const char *>() && !routeDoc["route_short_name"].is<const char *>())) {
    //     continue;
    // }

    // Route route;
    // route.agencyId = routeDoc["agency"]["onestop_id"].as<String>(); // "o-9q5c-bigbluebus", ...
    // route.id = routeDoc["onestop_id"].as<String>(); // "r-9q5c8-1", "r-9q5c8-2", "r-9q5c8-8", ...
    // if (!routeDoc["route_short_name"].is<const char*>()) {
    //   route.name = truncateRoute(routeDoc["route_long_name"].as<String>());
    // } else {
    //   route.name = truncateRoute(routeDoc["route_short_name"].as<String>());
    // }

    // String lineColorHex = routeDoc["route_color"].as<String>();
    // String textColorHex = routeDoc["route_text_color"].as<String>();
    // route.lineColor = (uint32_t)strtol(lineColorHex.c_str(), NULL, 16);
    // route.textColor = (uint32_t)strtol(textColorHex.c_str(), NULL, 16);

    // m_routes.push_back(m_routeTable->addRoute(route));
  // }
}

// The function that performs the truncation based on the specified rules.
String Stop::truncateName(const String &name, const bool truncateDowntown) {
  // We will work on a copy of the name so we don't modify the original reference.
  String result = name;

  // --- Rule 1: Cut off any line number / service and a dash at the beginning ---
  // This rule uses a combined heuristic to decide whether to truncate.
  int dashIndex = result.indexOf(" - ");

  // Only proceed if a dash is found.
  if (dashIndex != -1) {
    bool shouldTruncate = false;

    // Heuristic A: Check if the prefix is purely numeric (e.g., "2 - ...")
    String prefix = result.substring(0, dashIndex);
    prefix.trim(); // Remove whitespace for accurate checking
    
    bool prefixIsNumericOnly = true;
    if (prefix.length() > 0) {
      for (int i = 0; i < prefix.length(); i++) {
        if (!isDigit(prefix.charAt(i))) {
          prefixIsNumericOnly = false;
          break;
        }
      }
    } else {
      prefixIsNumericOnly = false;
    }
    
    if (prefixIsNumericOnly) {
      shouldTruncate = true;
    }

    // Heuristic B: Check if it looks like a long headsign (e.g., "... Downtown ... Station")
    // This is a fallback for non-numeric service names like "Metro E Line - ..."
    if (!shouldTruncate && (result.indexOf("Downtown") != -1 || result.indexOf("Station") != -1)) {
      shouldTruncate = true;
    }

    // If either heuristic passed, perform the truncation.
    if (shouldTruncate) {
      result = result.substring(dashIndex + 3); // Length of " - " is 3
    }
  }

  // Trim whitespace after every operation to keep the string clean.
  result.trim();

  // --- Rule 2: Cut off "Downtown" at the beginning ---
  if (truncateDowntown && result.startsWith("Downtown")) {
    // Take the substring that starts after the word "Downtown".
    result = result.substring(String("Downtown").length());
  }

  result.trim();

  // --- Rule 3: Cut off "Station" at the end ---
  if (result.endsWith("Station")) {
    // Take the substring from the beginning up to where "Station" starts.
    result = result.substring(0, result.length() - String("Station").length());
  }

  // Perform a final trim to clean up any trailing space and return the result.
  result.trim();
  return result;
}

String Stop::truncateRoute(const String &line) {
  String result = line;

  // --- Cut off "Metro" at the beginning ---
  if (result.startsWith("Metro")) {
    // Take the substring that starts after the word "Metro".
    result = result.substring(String("Metro").length());
  }

  result.trim();

  // --- Rule 3: Cut off "Line" at the end ---
  if (result.endsWith("Line")) {
    // Take the substring from the beginning up to where "Line" starts.
    result = result.substring(0, result.length() - String("Line").length());
  }

  // Perform a final trim to clean up any trailing space and return the result.
  result.trim();
  return result;
}