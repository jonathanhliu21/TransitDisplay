#include "Stop.h"

#include <cstdlib>
#include <vector>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "constants.h"
#include "RouteTable.h"
#include "arduino_secrets.h"

Stop::Stop(const String &oneStopId, const String &name, const String &feedId, const int &numDepartures, RouteTable *routeTable, HTTPClient *client)
  : m_id{ oneStopId }, m_numDepartures{ numDepartures }, m_feedId{feedId}, m_routeTable{ routeTable }, m_client{ client },
    m_lastRetrieveTime{ 0 } {
  m_name = truncateName(name, false);  // Don't truncate "Downtown" when retrieving station name
  m_departures.reserve(m_numDepartures);
  m_departures.resize(m_numDepartures);
}

// getters
String Stop::getId() const {
  return m_id;
}
int Stop::getNumDepartures() const {
  return m_numDepartures;
}
String Stop::getName() const {
  return m_name;
}
String Stop::getFeedId() const {
  return m_feedId;
}
const std::vector<Departure> *Stop::getDepartures() const {
  return &m_departures;
}

void Stop::debugPrintStop() const {
  Serial.print(F("  ID: "));
  Serial.println(m_id);
  
  Serial.print(F("  Name: "));
  Serial.println(m_name);

  Serial.print(F("  Feed ID: "));
  Serial.println(m_feedId);

  Serial.println(F("-----------------------"));
}

void Stop::callDeparturesAPI() {
  String endpoint = String(STOPS_ENDPOINT_PREFIX) + "/departures/" + m_id + "/departures?api_key" + SECRET_API_KEY + "&limit=" + m_numDepartures;

  int loopCnt = 0;
  // for "next" pages
  // while (endpoint != "" && loopCnt < MAX_PAGES_PROCESSED) {
  //   // Get the status code from the server's response
  //   int statusCode = m_client->responseStatusCode();
  //   if (statusCode != 200) {
  //     Serial.println("Status Code for stop did not return 200");
  //     return false;
  //   }
  // }
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
