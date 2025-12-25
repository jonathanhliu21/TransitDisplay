#include "BaseRetriever.h"
#include "RouteRetriever.h"

#include <string>
#include <ArduinoJson.h>

#include "TransitTypes.h"
#include "Constants.h"
#include "Whitelist.h"

namespace
{
  const int ROUTE_MAX_PAGES_PROCESSED = Constants::MAX_PAGES_PROCESSED;
  const char *ROUTE_KEY_NAME = "routes";
  const char *ROUTES_ENDPOINT_PREFIX = "/api/v2/rest/routes";
}

RouteRetriever::RouteRetriever(APICaller *caller, float lat, float lon, float radius)
    : BaseRetriever{
          caller,
          constructEndpointString(lat, lon, radius),
          Constants::MAX_PAGES_PROCESSED, Constants::ROUTE_ERROR_PIN}
{
}

RouteRetriever::RouteRetriever(
    APICaller *caller, float lat, float lon, float radius, const Whitelist &whiteList)
    : BaseRetriever{
          caller,
          constructEndpointString(lat, lon, radius),
          Constants::MAX_PAGES_PROCESSED, Constants::ROUTE_ERROR_PIN},
      m_whitelist{whiteList}
{
}

/**
 * Retrieves routes
 *
 * @note CLEARS the route list
 */
bool RouteRetriever::retrieve()
{
  m_routeList.clear();
  JsonDocument filter = constructFilter();
  return loopRequest(filter, ROUTE_KEY_NAME);
}

RouteList RouteRetriever::getRouteList()
{
  return m_routeList;
}

Whitelist RouteRetriever::getWhiteList() const
{
  return m_whitelist;
}

void RouteRetriever::setWhiteList(const Whitelist &whitelist)
{
  m_whitelist = whitelist;
}

void RouteRetriever::parseOneElement(JsonVariantConst &routeDoc)
{
  if (!routeDoc["onestop_id"].is<const char *>() || routeDoc["agency"].isNull() || !routeDoc["agency"]["onestop_id"].is<const char *>() || (!routeDoc["route_long_name"].is<const char *>() && !routeDoc["route_short_name"].is<const char *>()))
  {
    return;
  }

  Route route;
  route.agencyOnestopId = routeDoc["agency"]["onestop_id"].as<std::string>(); // "o-9q5c-bigbluebus", ...

  // check if route is in whitelist
  if (m_whitelist.isActive() && !m_whitelist.inWhitelist(route.agencyOnestopId))
    return;

  // get route name and onestop ID
  route.onestopId = routeDoc["onestop_id"].as<std::string>(); // "r-9q5c8-1", "r-9q5c8-2", "r-9q5c8-8", ...
  if (!routeDoc["route_short_name"].is<const char *>())
  {
    route.name = routeDoc["route_long_name"].as<std::string>();
  }
  else
  {
    route.name = routeDoc["route_short_name"].as<std::string>();
  }

  // get route color
  std::string lineColorHex = routeDoc["route_color"].as<std::string>();
  std::string textColorHex = routeDoc["route_text_color"].as<std::string>();
  route.lineColor = std::stoi(lineColorHex, nullptr, 16);
  route.textColor = std::stoi(textColorHex, nullptr, 16);

  m_routeList.addRoute(route);
}

JsonDocument RouteRetriever::constructFilter() const
{
  JsonDocument filter;
  filter["meta"]["next"] = true;

  JsonObject filter_routes_0 = filter["routes"].add<JsonObject>();
  filter_routes_0["agency"]["onestop_id"] = true;
  filter_routes_0["onestop_id"] = true;
  filter_routes_0["route_color"] = true;
  filter_routes_0["route_text_color"] = true;
  filter_routes_0["route_short_name"] = true;
  filter_routes_0["route_long_name"] = true;

  return filter;
}

std::string RouteRetriever::constructEndpointString(float lat, float lon, float radius) const
{
  return std::string(ROUTES_ENDPOINT_PREFIX) + "?lat=" + std::to_string(lat) + "&lon=" + std::to_string(lon) + "&radius=" + std::to_string(radius);
}
