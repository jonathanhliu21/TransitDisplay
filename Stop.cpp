#include "Stop.h"

#include <cstdlib>
#include <ctime>
#include <cstring>
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
time_t Stop::timegm_custom(struct tm *timeinfo) {
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

Stop::Stop(const String &oneStopId, const String &name, const String &feedId, const int &numDepartures, RouteTable *routeTable)
  : m_id{ oneStopId }, m_numDepartures{ numDepartures }, m_feedId{feedId}, m_routeTable{ routeTable },
    m_lastRetrieveTime{ 0 } {
  m_name = name;  // Don't truncate "Downtown" when retrieving station name
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

bool Stop::callDeparturesAPI(std::time_t curTime) {
  String endpoint = String(STOPS_ENDPOINT_PREFIX) + "/" + m_id + "/departures?api_key=" + SECRET_API_KEY + "&limit=" + m_numDepartures + "&next=" + NEXT_N_SECONDS;
  const char* keys[] = {"Transfer-Encoding"};

  int loopCnt = 0;
  int numDeparturesProcessed = 0;
  // for "next" pages
  while (endpoint != "" && loopCnt < MAX_PAGES_PROCESSED) {
    HTTPClient client;
    client.collectHeaders(keys, 1);
    client.setTimeout(HTTP_TIMEOUT);

    // m_client->useHTTP10(true);
    // m_client->begin(TRANSIT_LAND_SERVER, 443, endpoint);
    client.begin(TRANSIT_LAND_SERVER, TRANSIT_LAND_PORT, endpoint, TRANSIT_LAND_ROOT_CERTIFICATE);
    // m_client->begin(*m_wifiClient, TRANSIT_LAND_SERVER, 443, endpoint, true);
    int httpCode = client.GET();

    if (httpCode != 200) {
      Serial.print("Departure from " + m_name + " Http request failed with code: ");
      Serial.println(httpCode);
      client.end();
      return false;
    }

    // Serial.println(endpoint);

    // Get the raw and the decoded stream
    Stream& rawStream = client.getStream();
    ChunkDecodingStream decodedStream(client.getStream());
    // Choose the right stream depending on the Transfer-Encoding header
    Stream& response =
        client.header("Transfer-Encoding") == "chunked" ? decodedStream : rawStream;

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
    // filter_stops_0["parent"]["stop_id"] = true;

    JsonDocument responseDoc;

    // String response = m_client->getString();
    // Serial.println(response);
    DeserializationError error = deserializeJson(responseDoc, response, DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(11));

    if (error) {
      Serial.println("Deserialization for departures from " + m_name + " failed");
      Serial.println(error.c_str());
      client.end();
      return false;
    }

    // m_client->end();
    // return false;


    if (responseDoc["meta"].isNull()
        || !responseDoc["meta"]["next"].is<const char*>()) {
      endpoint = "";
    } else {
      endpoint = responseDoc["meta"]["next"].as<String>();
      endpoint = endpoint.substring(strlen(TRANSIT_LAND_HTTPS));
    }

    // find if routes exist
    if (responseDoc["stops"].isNull()) {
      client.end();
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
        departure.direction = headsign;

        // Serial.println("here 6");

        // route and agency ID
        if (departureDoc["trip"]["route"].isNull() ||
            departureDoc["trip"]["route"]["onestop_id"].isNull()) continue;
        String onestopId = departureDoc["trip"]["route"]["onestop_id"].as<String>();
        if (m_routeTable->getRoute(onestopId) == nullptr) continue;
        departure.route = m_routeTable->getRoute(onestopId);
        if (departureDoc["trip"]["route"]["agency"].isNull() || departureDoc["trip"]["route"]["agency"]["onestop_id"].isNull()) continue;
        departure.agency_id = departureDoc["trip"]["route"]["agency"]["onestop_id"].as<String>();

        // Serial.println("here 7");

        // timestamp and delay
        if (departureDoc["departure"].isNull()) continue;
        String timestamp;
        String timestampScheduled;
        if (!departure.isRealTime) {
          // Serial.println("not real time");
          // Serial.println(departureDoc["departure"].size());
          if (departureDoc["departure"]["scheduled_utc"].isNull()) continue;
          timestamp = departureDoc["departure"]["scheduled_utc"].as<String>();
          departure.delay = 0;
        } else {
          // Serial.println("real time");
          if (departureDoc["departure"]["estimated_utc"].isNull()) {
            // some scheduled departures dont have estimated times
            departure.isRealTime = false;
            if (departureDoc["departure"]["scheduled_utc"].isNull()) continue;
            timestamp = departureDoc["departure"]["scheduled_utc"].as<String>();
            departure.delay = 0;
          } else {
            if (departureDoc["departure"]["estimated_delay"].isNull()) {
              timestampScheduled = departureDoc["departure"]["scheduled_utc"].as<String>();
            } else {
              departure.delay = departureDoc["departure"]["estimated_delay"].as<int>();
            }
            timestamp = departureDoc["departure"]["estimated_utc"].as<String>();
          }
        }

        std::tm timeinfo;
        strptime(timestamp.c_str(), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
        departure.timestamp = timegm_custom(&timeinfo);

        // calculate dely if not available
        if (timestampScheduled != "" && timestampScheduled != "null") {
          std::tm timeinfo2;
          strptime(timestampScheduled.c_str(), "%Y-%m-%dT%H:%M:%SZ", &timeinfo2);
          time_t timestampScheduledT = timegm_custom(&timeinfo2);
          departure.delay = departure.timestamp - timestampScheduledT;
        }

        // don't include timestamps before curtime
        if (departure.timestamp < curTime - DELAY_CUTOFF) {
          continue;
        }

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

    client.end();
  }

  return loopCnt > 0;
}


