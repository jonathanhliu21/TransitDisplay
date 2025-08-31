#include "Bridge.h"

#include <algorithm>
#include <set>
#include <vector>
#include <TFT_eSPI.h>

#include "Overpass_Regular16.h"
#include "TransitZone.h"
#include "constants.h"

Bridge::Bridge(TFT_eSPI *tft) : m_tft{tft}, m_routeDisplay(tft), m_depDisplay(tft), m_lastTimeRoute(0), m_lastTimeDep(0), m_firstTimeRoute(true), m_firstTimeDep(true) {
  // Initialize the mutex. 
  m_departures_mutex = new std::mutex();
}

Bridge::~Bridge() {
  // Clean up the dynamically allocated mutex
  delete m_departures_mutex;
  // Ensure the task is stopped and deleted if the object is destroyed.
  stop();
}

void Bridge::syncTime(time_t startTimeMillis, time_t startTimeUTC) {
  // set time sync
  m_startTimeS = startTimeMillis / 1000;
  m_startTimeUTC = startTimeUTC;
}

// called in main thread
void Bridge::setZone(TransitZone *zone, time_t startTimeMillis, time_t startTimeUTC) {
  // stop any thread and kill mutexes (very disgraceful)
  // gemini does not approve of this
  stop();
  delete m_departures_mutex;
  m_departures_mutex = new std::mutex();
  m_departures.clear();

  syncTime(startTimeMillis, startTimeUTC);

  // Prevent creating multiple tasks if begin() is called more than once.
  if (m_retrieval_thread_handle != NULL) {
      return;
  }

  // get zone information
  m_zone = zone;
  m_zone->init(); // retrieves routes from API
  m_name = m_zone->getName();
  
  std::vector<Route *> rts = m_zone->getRoutes();
  sortRoutes(rts);
  m_routes.clear();
  for (int i = 0; i < rts.size(); i++) {
    m_routes.push_back(*rts[i]);
  }

  modifyRoutes();
  retrieveDepartures(); // retrieves departures from API

  // start thread
  xTaskCreate(
    retrieval_task_runner,    // Function to implement the task
    "DepartureRetrievalTask", // Name of the task
    8192,                     // Stack size in words
    this,                     // Task input parameter (pointer to this instance)
    1,                        // Priority of the task
    &m_retrieval_thread_handle // Task handle to keep track of created task
  );

  // clear screen and set title
  m_tft->fillScreen(TFT_BLACK);
  m_tft->loadFont(Overpass_Regular16); // Must match the .vlw file name
  m_tft->setTextSize(16);
  m_tft->setTextColor(TFT_WHITE, TFT_BLACK);
  m_tft->setTextDatum(TC_DATUM);
  m_tft->drawString(m_name, 240, 19);
  m_tft->unloadFont();

  // set routes & departures
  m_routeDisplay.setRoutes(m_routes);
  m_depDisplay.setDepartures(m_departures);
}

// called in main thread
void Bridge::loop() {
  // The lock_guard ensures that the mutex is locked for the entire scope of this block.
  // No other thread can access m_departures until this function finishes.
  std::lock_guard<std::mutex> lock(*m_departures_mutex);

  if (m_firstTimeRoute || millis() - m_lastTimeRoute >= ROUTE_CYCLE_TIME) {
    m_firstTimeRoute = false;
    m_lastTimeRoute = millis();
    m_routeDisplay.cycle(); 
  }

  if (m_firstTimeDep || millis() - m_lastTimeDep >= DEP_CYCLE_TIME) {
    m_firstTimeDep = false;
    m_lastTimeDep = millis();
    m_depDisplay.setDepartures(m_departures);
    m_depDisplay.cycle();
  }
}

// called in bg thread
void Bridge::retrieveDepartures() {
  // Serial.print("Current time: ");
  time_t curTime = retrieveTime();
  m_zone->updateDepartures(curTime);
  std::vector<Departure> departures = m_zone->getDepartures();
  curTime = retrieveTime();

  // threading code
  // The lock_guard ensures that the mutex is locked for the entire scope of this block.
  // No other thread can access m_departures until this function finishes.
  std::lock_guard<std::mutex> lock(*m_departures_mutex);

  m_departures.clear();
  for (Departure dep : departures) {
    if (!dep.isValid) {
      continue;
    }

    long long timeDelay = dep.delay;

    if (dep.agency_id == METRO_LOS_ANGELES) {
      if (timeDelay < -FIVE_DAYS) {
        timeDelay += SEVEN_DAYS;
      }
      if (timeDelay > FIVE_DAYS) {
        timeDelay -= SEVEN_DAYS;
      }
    }
    
    // Serial.println(timeDelay);
    
    dep = modifyDeparture(dep);
    
    BridgeDeparture bridgeDep;
    bridgeDep.direction = truncateStop(dep.direction, true);
    bridgeDep.routeColor = dep.route->lineColor;
    bridgeDep.textColor = dep.route->textColor;
    bridgeDep.line = truncateRoute(dep.route->name);
    bridgeDep.delayColor = getDelayColor(timeDelay, dep.isRealTime);
    bridgeDep.mins = (dep.timestamp - curTime) / 60;
    m_departures.push_back(bridgeDep);
  }
}

void Bridge::debugPrintRoutes() const {
  Serial.println(F("--- Route Info ---"));
  if (m_routes.size() == 0) {
    Serial.println("No routes found");
    return;
  }
  
  for (Route route: m_routes) {
    Serial.print(F("ID: "));
    Serial.println(route.id);

    Serial.print(F("Name: "));
    Serial.println(route.name);

    Serial.print(F("Line Color: 0x"));
    Serial.println(route.lineColor, HEX);

    Serial.print(F("Text Color: 0x"));
    Serial.println(route.textColor, HEX);

    Serial.print(F("Agency ID: "));
    Serial.println(route.agencyId);
    
    Serial.println(F("------------------"));
  }
}

void Bridge::debugPrintDepartures() const {
    // threading code
  // The lock_guard ensures that the mutex is locked for the entire scope of this block.
  // No other thread can access m_departures until this function finishes.
  std::lock_guard<std::mutex> lock(*m_departures_mutex);

  Serial.println(F("--- Departures Info ---"));
  if (m_departures.size() == 0) {
    Serial.println("No departures found");
    return;
  }

  for (BridgeDeparture dep : m_departures) {
    Serial.print(dep.textColor, HEX);
    Serial.print("\t");
    Serial.print(dep.routeColor, HEX);
    Serial.print("\t");
    Serial.print(dep.line);
    Serial.print("\t");
    Serial.print(dep.direction);
    Serial.print("\t\t");
    Serial.print(dep.mins);
    Serial.print("\t");
    Serial.println(dep.delayColor, HEX);
    Serial.println(F("------------------"));
  }
}

void Bridge::stop() {
  if (m_retrieval_thread_handle != NULL) {
    vTaskDelete(m_retrieval_thread_handle);
    m_retrieval_thread_handle = NULL; // Set handle to NULL to indicate task is stopped.
    Serial.println("Departure retrieval task stopped.");
  }
}

time_t Bridge::retrieveTime() const {
  return millis() / 1000 - m_startTimeS + m_startTimeUTC;
}

int Bridge::getDelayColor(int delay, bool isRealTime) const {
  if (!isRealTime) return NO_RT_INFO_COLOR;
  if (delay > DELAY_CUTOFF) return DELAYED_COLOR;
  if (delay < -DELAY_CUTOFF) return EARLY_COLOR;
  return ON_TIME_COLOR;
}

void Bridge::modifyRoutes() {
  // first combine directions
  auto cmp = [](const Route &a, const Route &b) {
    if (a.agencyId == b.agencyId) {
      return a.name < b.name;
    }
    return a.agencyId < b.agencyId;
  };
  std::set<Route, decltype(cmp)> unique_base_names(cmp);

  for (const Route& r : m_routes) {
    // Find the position of the last hyphen '-'.
    // Arduino's lastIndexOf() returns -1 if the character is not found.
    Route route = r;
    String line = r.name;
    int pos = line.lastIndexOf('-');

    if (pos != -1) {
        // Extract the suffix using the substring() method.
        String suffix = line.substring(pos + 1);

        // Check if the suffix is a valid cardinal direction.
        if (suffix == "N" || suffix == "S" || suffix == "E" || suffix == "W") {
            // It's a valid directional line, so extract the base name.
            route.name = line.substring(0, pos);
            unique_base_names.insert(route);
        } else {
            // Not a valid direction (e.g., "Red-Express"), so use the whole string.
            unique_base_names.insert(route);
        }
    } else {
        // No hyphen, so use the whole string.
        unique_base_names.insert(route);
    }
  }
  m_routes.assign(unique_base_names.begin(), unique_base_names.end());

  // truncate names
  for (int i = 0; i < m_routes.size(); i++) {
    m_routes[i].name = truncateRoute(m_routes[i].name);
  }

  // color LA metro buses
  for (int i = 0; i < m_routes.size(); i++) {
    Route &route = m_routes[i];
    if (route.agencyId == METRO_LOS_ANGELES) {
      if (route.lineColor == 0 && route.textColor == 0xffffff) {
        route.lineColor = LA_METRO_RAPID_COLOR;
      }
      if (route.lineColor == 0 && route.textColor == 0) {
        route.lineColor = LA_METRO_LOCAL_COLOR;
        route.textColor = COLOR_WHITE;
      }
    }
  }


  // color buses
  for (int i = 0; i < m_routes.size(); i++) {
    Route &route = m_routes[i];
    if (route.lineColor == 0 && route.textColor == 0xffffff) {
      route.lineColor = LA_METRO_RAPID_COLOR;
    }
    if (route.lineColor == 0 && route.textColor == 0) {
      route.lineColor = 0;
      route.textColor = COLOR_WHITE;
    }
  }
}

Departure Bridge::modifyDeparture(const Departure &dep) {
  Departure res = dep;
  if (res.agency_id == METRO_LOS_ANGELES) {
    if (res.route->lineColor == 0 && res.route->textColor == 0xffffff) {
      res.route->lineColor = LA_METRO_RAPID_COLOR;
    }
    if (res.route->lineColor == 0 && res.route->textColor == 0) {
      res.route->lineColor = LA_METRO_LOCAL_COLOR;
      res.route->textColor = COLOR_WHITE;
    }
  }

  return res;
}

// The function that performs the truncation based on the specified rules.
String Bridge::truncateStop(const String &name, const bool truncateDowntown) const {
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
    if (!shouldTruncate && (result.indexOf("Downtown") != -1 || result.indexOf("Station") != -1) && !result.endsWith("Downtown")) {
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

  // Also cut off "Rapid" at the end
  if (result.endsWith("Rapid")) {
    // Take the substring from the beginning up to where "Rapid" starts.
    result = result.substring(0, result.length() - String("Rapid").length());
  }

  // Perform a final trim to clean up any trailing space and return the result.
  result.trim();
  return result;
}

String Bridge::truncateRoute(const String &routeStr) const {
  String result = routeStr;

  // --- Cut off "Metro" at the beginning ---
  if (result.startsWith("Metro")) {
    // Take the substring that starts after the word "Metro".
    result = result.substring(String("Metro").length());
  }

  result.trim();

  // --- Cut off "Line" at the end ---
  if (result.endsWith("Line")) {
    // Take the substring from the beginning up to where "Line" starts.
    result = result.substring(0, result.length() - String("Line").length());
  }

  // --- Truncate stuff like "J Line formerly Silver Line with services 910 and 950 to Harbor Gateway and San Pedro respectively" ---
  // Also avoids incorrectly shortening names like "Rapid 6".
  int firstSpaceIndex = result.indexOf(' ');

  // Only check if a space exists.
  if (firstSpaceIndex != -1) {
    bool shouldTruncate = false;
    
    // Get the parts before and after the first space.
    String prefix = result.substring(0, firstSpaceIndex);
    String suffix = result.substring(firstSpaceIndex);
    suffix.trim(); // Clean up suffix for inspection.

    // Heuristic A (New): If the first word is a single letter or digit, truncate.
    if (prefix.length() == 1 && (isAlpha(prefix.charAt(0)) || isDigit(prefix.charAt(0)))) {
        shouldTruncate = true;
    }

    // Heuristic B (Old): If not, truncate only if the suffix is "complex".
    if (!shouldTruncate) {
        bool isComplex = false;
        // Check if the suffix contains anything other than digits.
        for (int i = 0; i < suffix.length(); i++) {
          if (!isDigit(suffix.charAt(i))) {
            isComplex = true; // Found a non-digit character, so it's complex.
            break;
          }
        }
        if (isComplex) {
            shouldTruncate = true;
        }
    }

    // If either heuristic decided we should truncate, do it.
    if (shouldTruncate) {
      result = prefix; // result becomes just the prefix
    }
  }

  // Perform a final trim to clean up any trailing space and return the result.
  result.trim();
  return result;
}


void Bridge::sortRoutes(std::vector<Route *> &routes) const {
  std::sort(routes.begin(), routes.end(), [](Route * const &a, Route * const &b) {
    if (a->agencyId == b->agencyId) {
      return a->name < b->name;
    }
    return a->agencyId < b->agencyId;
  });
}

void Bridge::retrieval_loop() {
  unsigned long last_retrieval_time = 0;
  bool firstTime = true;
  while (true) {
    // Check if it's time to run the retrieval logic.
    // Serial.print("millis: ");
    // Serial.println(millis());
    if (millis() - last_retrieval_time >= REFRESH_PERIOD || firstTime) {
      // Serial.println("here");
      // Update the time *before* calling the function. This handles the case
      // where the function takes a long time to execute.
      last_retrieval_time = millis();
      firstTime = false;

      // Serial.println("Retrieving...");
      retrieveDepartures();
      // debugPrintDepartures();

      // After the function returns, the loop will continue and the next
      // check will occur. If retrieveDepartures() took more than 60 seconds,
      // the condition `millis() - last_retrieval_time >= RETRIEVAL_INTERVAL_MS`
      // will immediately be true again on the next iteration.
    }

    // A small delay to prevent the loop from busy-waiting and starving other tasks.
    // This yields control to the FreeRTOS scheduler. 10ms is a reasonable value.
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void Bridge::retrieval_task_runner(void* pvParameters) {
    Bridge* bridge_instance = static_cast<Bridge*>(pvParameters);
    bridge_instance->retrieval_loop();
}
