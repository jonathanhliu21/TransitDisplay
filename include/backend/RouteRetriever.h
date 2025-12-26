#ifndef ROUTE_RETRIEVER_H
#define ROUTE_RETRIEVER_H

#include <ArduinoJson.h>
#include <string>

#include "backend/APICaller.h"
#include "backend/BaseRetriever.h"
#include "types/RouteList.h"
#include "types/Whitelist.h"

class RouteRetriever : public BaseRetriever
{
public:
  RouteRetriever(APICaller *caller, float lat, float lon, float radius);
  RouteRetriever(APICaller *caller, float lat, float lon, float radius, const Whitelist &whiteList);

  virtual bool retrieve() override;
  RouteList getRouteList() const;

  Whitelist getWhiteList() const;
  void setWhiteList(const Whitelist &whitelist);

protected:
  virtual void parseOneElement(JsonVariantConst &doc) override;

private:
  RouteList m_routeList;
  Whitelist m_whitelist;

  JsonDocument constructFilter() const;
  std::string constructEndpointString(float lat, float lon, float radius) const;
};

#endif