#ifndef STOP_RETREIVER_H
#define STOP_RETRIEVER_H

#include <ArduinoJson.h>
#include <string>

#include "StopList.h"
#include "APICaller.h"
#include "BaseRetriever.h"
#include "Whitelist.h"

class StopRetriever : public BaseRetriever
{
public:
  StopRetriever(APICaller *caller, float lat, float lon, float radius);
  StopRetriever(APICaller *caller, float lat, float lon, float radius, const Whitelist &whiteList);

  virtual bool retrieve() override;
  StopList getStopList() const;

  Whitelist getWhiteList() const;
  void setWhiteList(const Whitelist &whitelist);

protected:
  virtual void parseOneElement(JsonVariantConst &doc) override;

private:
  float m_lat, m_lon, m_radius;
  StopList m_stopList;
  Whitelist m_whitelist;

  JsonDocument constructFilter() const;
  std::string constructEndpointString(float lat, float lon, float radius) const;
  std::string constructEndpointString(float lat, float lon, float radius, const Whitelist &wl) const;
};

#endif