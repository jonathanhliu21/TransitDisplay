#include "Stop.h"

#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <StreamUtils.h>
#include <stdlib.h> // For getenv, setenv, and unsetenv

#include "constants.h"
#include "RouteTable.h"
#include "arduino_secrets.h"


// A portable implementation of timegm()
time_t timegm_custom(struct tm *timeinfo) {
  time_t result;
  
  // 1. Save the current TZ environment variable
  char *tz_original = getenv("TZ");
  
  // 2. Set the timezone to UTC
  setenv("TZ", "", 1); // An empty string or "UTC0" means UTC
  tzset(); // Apply the new timezone
  
  // 3. Call mktime(), which now thinks the struct is in UTC
  result = mktime(timeinfo);
  
  // 4. Restore the original timezone
  if (tz_original) {
    setenv("TZ", tz_original, 1);
  } else {
    unsetenv("TZ");
  }
  tzset(); // Apply the original timezone back
  
  return result;
}

Stop::Stop(const String &oneStopId, const String &name, const String &feedId, const int &numDepartures, RouteTable *routeTable, HTTPClient *client)
  : m_id{ oneStopId }, m_numDepartures{ numDepartures }, m_feedId{feedId}, m_routeTable{ routeTable }, m_client{ client },
    m_lastRetrieveTime{ 0 } {
  m_name = truncateName(name, false);  // Don't truncate "Downtown" when retrieving station name
  m_departures.reserve(m_numDepartures);
  m_departures.resize(m_numDepartures);

  for (int i = 0 ; i < m_numDepartures; i++) {
    m_departures[i].isValid = false;
  }
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
std::vector<Departure> Stop::getDepartures() const {
  return m_departures;
}

void Stop::debugPrintStop() const {
  Serial.print(F("  ID: "));
  Serial.println(m_id);
  
  // Check if there are any departures to print
  if (m_departures.empty()) {
    Serial.println(F("    No departures found."));
    return;
  }

  // Iterate over each departure in the vector
  for (const auto& departure : m_departures) {
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

bool Stop::callDeparturesAPI() {
  String endpoint = String(STOPS_ENDPOINT_PREFIX) + "/" + m_id + "/departures?api_key=" + SECRET_API_KEY + "&limit=" + m_numDepartures + "&next=" + NEXT_N_SECONDS;
  const char* keys[] = {"Transfer-Encoding"};

  int loopCnt = 0;
  int numDeparturesProcessed = 0;
  // for "next" pages
  while (endpoint != "" && loopCnt < MAX_PAGES_PROCESSED) {
    m_client->collectHeaders(keys, 1);

    // m_client->useHTTP10(true);
    // m_client->begin(TRANSIT_LAND_SERVER, 443, endpoint);
    m_client->begin(TRANSIT_LAND_SERVER, TRANSIT_LAND_PORT, endpoint, TRANSIT_LAND_ROOT_CERTIFICATE);
    // m_client->begin(*m_wifiClient, TRANSIT_LAND_SERVER, 443, endpoint, true);
    m_client->GET();

    // Serial.println(endpoint);

    // Get the raw and the decoded stream
    Stream& rawStream = m_client->getStream();
    ChunkDecodingStream decodedStream(m_client->getStream());
    // Choose the right stream depending on the Transfer-Encoding header
    Stream& response =
        m_client->header("Transfer-Encoding") == "chunked" ? decodedStream : rawStream;

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
    // filter_stops_0["parent"]["stop_id"] = true;

    JsonDocument responseDoc;

    // String response = m_client->getString();
    // Serial.println(response);
    DeserializationError error = deserializeJson(responseDoc, response, DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(11));

    if (error) {
      Serial.println("Deserialization for departures failed");
      Serial.println(error.c_str());
      m_client->end();
      return false;
    }

    // m_client->end();
    // return false;


    if (responseDoc["meta"].isNull()
        || !responseDoc["meta"]["next"].is<const char*>()) {
      endpoint = "";
    } else {
      endpoint = responseDoc["meta"]["next"].as<String>();
    }

    // find if routes exist
    if (responseDoc["stops"].isNull()) {
      m_client->end();
      Serial.println("stops key is not there");
      return false;
    }

    JsonArrayConst arr = responseDoc["stops"].as<JsonArrayConst>();
    int size = arr.size();

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

      // Serial.println("here 1");

      // check that the location type is an actual stop and not some random exit
      if (stopInfo["location_type"].isNull() || stopInfo["location_type"].as<int>() != 0) {
        continue;
      }

      // Serial.println("here 2");

      // extract info from stop
      // departures
      if (!stopInfo["departures"].is<JsonArrayConst>() || stopInfo["departures"].size() <= 0) {
        continue;
      }

      // Serial.println("here 3");

      JsonArrayConst departuresArr = stopInfo["departures"].as<JsonArrayConst>();
      int departuresArrSize = departuresArr.size();
      for (int j = 0; j < departuresArrSize; j++) {
        if (numDeparturesProcessed >= m_numDepartures) break;

        // Serial.println("here 4");

        if (departuresArr[j].isNull() || !departuresArr[j].is<JsonObjectConst>()) continue;
        JsonVariantConst departureDoc = departuresArr[j];

        // Serial.println("here 5");

        if (departureDoc["trip"].isNull()) continue;

        Departure departure;
        departure.isValid = true;
        departure.isRealTime = true;
        if (departureDoc["schedule_relationship"].isNull() && (departureDoc["trip"]["schedule_relationship"].isNull())) {
          departure.isRealTime = false;
        } else {
          // schedule relationship
          String schedule_relationship;
          if (departureDoc["schedule_relationship"].isNull()) {
            schedule_relationship = departureDoc["trip"]["schedule_relationship"].as<String>();
          } else {
            schedule_relationship = departureDoc["schedule_relationship"].as<String>();
          }
          if (schedule_relationship == "STATIC" || schedule_relationship == "NO_DATA") {
            departure.isRealTime = false;
          }
          
          if (schedule_relationship == "DELETED" || schedule_relationship == "SKIPPED" || schedule_relationship == "CANCELED") continue;
        }

        // headsign
        String headsign;
        if (departureDoc["trip"]["trip_headsign"].isNull()) {
          if (departureDoc["stop_headsign"].isNull()) continue;
          headsign = departureDoc["stop_headsign"].as<String>();
        } else {
          headsign = departureDoc["trip"]["trip_headsign"].as<String>();
        }
        departure.direction = truncateName(headsign, true); // truncate downtown this time

        // Serial.println("here 6");

        // route and agency ID
        if (departureDoc["trip"]["route"].isNull() ||
            departureDoc["trip"]["route"]["onestop_id"].isNull()) continue;
        String onestopId = departureDoc["trip"]["route"]["onestop_id"].as<String>();
        if (m_routeTable->getRoute(onestopId) == nullptr) continue;
        departure.route = m_routeTable->getRoute(onestopId);
        departure.agency_id = onestopId;

        // Serial.println("here 7");

        // timestamp and delay
        if (departureDoc["departure"].isNull()) continue;
        String timestamp;
        if (!departure.isRealTime) {
          // Serial.println("not real time");
          // Serial.println(departureDoc["departure"].size());
          if (departureDoc["departure"]["scheduled_utc"].isNull()) continue;
          timestamp = departureDoc["departure"]["scheduled_utc"].as<String>();
          departure.delay = 0;
        } else {
          // Serial.println("real time");
          if (departureDoc["departure"]["estimated_delay"].isNull() || departureDoc["departure"]["estimated_utc"].isNull()) {
            // some scheduled departures dont have estimated times
            departure.isRealTime = false;
            if (departureDoc["departure"]["scheduled_utc"].isNull()) continue;
            timestamp = departureDoc["departure"]["scheduled_utc"].as<String>();
            departure.delay = 0;
          } else {
            timestamp = departureDoc["departure"]["estimated_utc"].as<String>();
            departure.delay = departureDoc["departure"]["estimated_delay"].as<int>();
          }
        }
        std::tm timeinfo;
        strptime(timestamp.c_str(), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
        departure.timestamp = timegm_custom(&timeinfo);

        // Serial.println("here 8");

        if (numDeparturesProcessed >= m_departures.size()) {
          Serial.println("somehow departures processed >= size of departures!!!!");
          break;
        }

        m_departures[numDeparturesProcessed] = departure;
        numDeparturesProcessed++;
        loopCnt++;
      }
    }

    // Serial.print("Num processed: ");
    // Serial.println(numDeparturesProcessed);

    // mark any unused slots in departures array as invalid
    for (int i = numDeparturesProcessed; i < m_numDepartures; i++) {
      m_departures[i].isValid = false;
    }

    m_client->end();
  }

  return loopCnt > 0;
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
