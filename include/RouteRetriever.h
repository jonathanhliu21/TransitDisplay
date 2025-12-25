#ifndef ROUTE_RETRIEVER_H
#define ROUTE_RETRIEVER_H

#include <ArduinoJson.h>
#include <vector>
#include <string>

#include "RouteList.h"
#include "APICaller.h"
#include "BaseRetriever.h"
#include "Whitelist.h"

class RouteRetriever : public BaseRetriever
{
public:
  RouteRetriever(APICaller *caller, float lat, float lon, float radius);
  RouteRetriever(APICaller *caller, float lat, float lon, float radius, const Whitelist &whiteList);

  virtual bool retrieve() override;
  RouteList getRouteList();

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