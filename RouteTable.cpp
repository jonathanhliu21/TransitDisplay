#include "RouteTable.h"

#include <vector>
#include <algorithm>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <StreamUtils.h>

#include "arduino_secrets.h"
#include "constants.h"

RouteTable::~RouteTable() {
  for (int i = 0; i < m_routes.size(); i++) {
    if (m_routes[i] != nullptr) {
      delete m_routes[i];
      m_routes[i] = nullptr;
    }
  }
}

std::vector<Route*> RouteTable::retrieveRoutes(float lat, float lon, float radius, std::vector<String> *whiteList) {
  String endpoint = String(ROUTES_ENDPOINT_PREFIX) + "?api_key=" + SECRET_API_KEY + "&lat=" + String(lat, 6) + "&lon=" + String(lon, 6) + "&radius=" + radius;
  int loopCnt = 0;
  std::vector<Route*> routes;
  const char* keys[] = {"Transfer-Encoding"};

  // for "next" pages
  while (endpoint != "" && loopCnt < MAX_PAGES_PROCESSED) {
    HTTPClient client;
    // Serial.print("endpoint: ");
    // Serial.println(endpoint);
    client.collectHeaders(keys, 1);

    // m_client->begin(TRANSIT_LAND_SERVER, 443, endpoint);
    client.begin(TRANSIT_LAND_SERVER, TRANSIT_LAND_PORT, endpoint, TRANSIT_LAND_ROOT_CERTIFICATE);
    // m_client->begin(*m_wifiClient, TRANSIT_LAND_SERVER, 443, endpoint, true);
    client.GET();

    // String responseStr = m_client->getString();
    // Serial.println(responseStr);

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
    DeserializationError error = deserializeJson(responseDoc, response, DeserializationOption::Filter(filter));
    if (error) {
      Serial.println("Deserialization for route failed");
      Serial.println(error.c_str());
      client.end();
      return routes;
    }
    if (responseDoc["meta"].isNull()
        || !responseDoc["meta"]["next"].is<const char*>()) {
      endpoint = "";
    } else {
      endpoint = responseDoc["meta"]["next"].as<String>();
    }

    // find if routes exist
    if (responseDoc["routes"].isNull()) {
      client.end();
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
      if (arr[i].isNull()) {
        continue;
      }
      // Serial.println("Crash 4");
      JsonVariantConst routeDoc = arr[i].as<JsonVariantConst>();
      if (!routeDoc["onestop_id"].is<const char*>()
          || routeDoc["agency"].isNull()
          || !routeDoc["agency"]["onestop_id"].is<const char*>()
          || (!routeDoc["route_long_name"].is<const char *>() && !routeDoc["route_short_name"].is<const char *>())) {
          continue;
      }

      // Serial.println("Crash 4.1");

      Route route;
      route.agencyId = routeDoc["agency"]["onestop_id"].as<String>(); // "o-9q5c-bigbluebus", ...

      if (whiteList != nullptr && std::find(whiteList->begin(), whiteList->end(), route.agencyId) == whiteList->end()) {
        continue;
      }

      route.id = routeDoc["onestop_id"].as<String>(); // "r-9q5c8-1", "r-9q5c8-2", "r-9q5c8-8", ...
      if (!routeDoc["route_short_name"].is<const char*>()) {
        route.name = routeDoc["route_long_name"].as<String>();
      } else {
        route.name = routeDoc["route_short_name"].as<String>();
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

    client.end();
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
  
  m_routes.push_back(newRoute);
  return newRoute;
}

