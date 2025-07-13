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
    m_isValidStop{ false }, m_lastRetrieveTime{ 0 } {
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

void Stop::init() {
  // Test cases

  m_isValidStop = retrieveStopInfo();
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

  return true;
}


// The function that performs the truncation based on the specified rules.
String Stop::truncateName(const String &name, const bool truncateDowntown) const {
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
