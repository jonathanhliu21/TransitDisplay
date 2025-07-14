#include "RouteTable.h"

#include <vector>
#include <ArduinoJson.h>

#include "arduino_secrets.h"
#include "constants.h"

RouteTable::RouteTable(HttpClient *client) : m_client(client) {}

RouteTable::~RouteTable() {
  for (int i = 0; i < m_routes.size(); i++) {
    if (m_routes[i] != nullptr) {
      delete m_routes[i];
      m_routes[i] = nullptr;
    }
  }
}

std::vector<Route*> RouteTable::retrieveRoutes(float lat, float lon, float radius) {
  String endpoint = String(ROUTES_ENDPOINT_PREFIX) + "?api_key=" + SECRET_API_KEY + "&lat=" + String(lat, 6) + "&lon=" + String(lon, 6) + "&radius=" + radius;
  int loopCnt = 0;
  std::vector<Route*> routes;

  // for "next" pages
  while (endpoint != "" && loopCnt < 5) {
    // Serial.print("endpoint: ");
    // Serial.println(endpoint);

    m_client->get(endpoint);

    // Get the status code from the server's response
    int statusCode = m_client->responseStatusCode();
    if (statusCode != 200) {
      Serial.println("Status Code for stop did not return 200");
      return routes;
    }

    // Create filter
    JsonDocument filter;
    filter["meta"]["next"] = true;

    JsonObject filter_routes_0 = filter["routes"].add<JsonObject>();
    filter_routes_0["agency"]["onestop_id"] = true;
    filter_routes_0["onestop_id"] = true;
    filter_routes_0["route_color"] = true;
    filter_routes_0["route_text_color"] = true;
    filter_routes_0["route_short_name"] = true;
    filter_routes_0["route_long_name"] = true;

    // Store response
    JsonDocument responseDoc;
    // Serial.println(m_client->responseBody());
    DeserializationError error = deserializeJson(responseDoc, m_client->responseBody(), DeserializationOption::Filter(filter));
    if (error) {
      Serial.println("Deserialization for stop failed");
      Serial.println(error.c_str());
      return routes;
    }
    if (!responseDoc["meta"].is<JsonVariantConst>()
        || !responseDoc["meta"]["next"].is<const char*>()) {
      endpoint = "";
    } else {
      endpoint = responseDoc["meta"]["next"].as<String>();
    }

    // find if routes exist
    if (!responseDoc["routes"].is<JsonArrayConst>()) {
      Serial.println("routes key is not there");
      return routes;
    }

    // Serial.println(responseDoc["routes"].as<JsonArrayConst>().size());
    // Serial.println("crash 2");

    JsonArrayConst arr = responseDoc["routes"].as<JsonArrayConst>();
    int size = arr.size();

    // Serial.println("crash 3");
    // Serial.println("size: " + String(size));

    for (int i = 0; i < size; i++) {
      if (!arr[i].is<JsonVariantConst>()) {
        continue;
      }
      // Serial.println("Crash 4");
      JsonVariantConst routeDoc = arr[i].as<JsonVariantConst>();
      if (!routeDoc["onestop_id"].is<const char*>()
          || !routeDoc["agency"].is<JsonVariantConst>()
          || !routeDoc["agency"]["onestop_id"].is<const char*>()
          || (!routeDoc["route_long_name"].is<const char *>() && !routeDoc["route_short_name"].is<const char *>())) {
          continue;
      }

      // Serial.println("Crash 4.1");

      Route route;
      route.agencyId = routeDoc["agency"]["onestop_id"].as<String>(); // "o-9q5c-bigbluebus", ...

      route.id = routeDoc["onestop_id"].as<String>(); // "r-9q5c8-1", "r-9q5c8-2", "r-9q5c8-8", ...
      if (!routeDoc["route_short_name"].is<const char*>()) {
        route.name = routeDoc["route_long_name"].as<String>();
      } else {
        route.name = truncateRoute(routeDoc["route_short_name"].as<String>());
      }

      // Serial.println("Crash 4.2");

      String lineColorHex = routeDoc["route_color"].as<String>();
      String textColorHex = routeDoc["route_text_color"].as<String>();
      route.lineColor = (uint32_t)strtol(lineColorHex.c_str(), NULL, 16);
      route.textColor = (uint32_t)strtol(textColorHex.c_str(), NULL, 16);

      // Serial.println("Crash 4.3");

      Route *rt = addRoute(route);
      routes.push_back(rt);

      // Serial.println("Crash 4.4");
      // Serial.println("Crash 4.5");
    }

    loopCnt++;
  }
  // Serial.println("crash 5");

  return routes;
}

Route *RouteTable::getRoute(const String &oneStopId) const {
  for (int i = 0; i < m_routes.size(); i++) {
    if (m_routes[i]->id == oneStopId) {
      return m_routes[i];
    }
  }

  return nullptr;
}

void RouteTable::debugPrintAllRoutes() const {
  Serial.println(F("--- Route Info ---"));
  if (m_routes.size() == 0) {
    Serial.println("No routes found");
    return;
  }
  
  for (Route *route: m_routes) {
    Serial.print(F("ID: "));
    Serial.println(route->id);

    Serial.print(F("Name: "));
    Serial.println(route->name);

    Serial.print(F("Line Color: 0x"));
    Serial.println(route->lineColor, HEX);

    Serial.print(F("Text Color: 0x"));
    Serial.println(route->textColor, HEX);

    Serial.print(F("Agency ID: "));
    Serial.println(route->agencyId);
    
    Serial.println(F("------------------"));
  }
}

Route* RouteTable::addRoute(const Route &route) {
  // find if route exists
  Route *curRoute = getRoute(route.id);
  if (curRoute != nullptr) return curRoute;

  Route *newRoute = new Route;
  newRoute->id = route.id;
  newRoute->name = route.name;
  newRoute->lineColor = route.lineColor;
  newRoute->textColor = route.textColor;
  newRoute->agencyId = route.agencyId;

  modifyRoute(*newRoute);
  
  m_routes.push_back(newRoute);
  return newRoute;
}

void RouteTable::modifyRoute(Route &route) const {
  route.name = truncateRoute(route.name);

  // color the LA metro buses
  if (route.agencyId == "o-9q5-metro~losangeles") {
    if (route.lineColor == 0 && route.textColor == 0xffffff) {
      route.lineColor = 0xC54858;
    }
    if (route.lineColor == 0 && route.textColor == 0) {
      route.lineColor = 0xfa7343;
      route.textColor = 0xffffff;
    }
  }
}

String RouteTable::truncateRoute(const String &routeStr) const {
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