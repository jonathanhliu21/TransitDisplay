#include <Arduino.h>
#include <WiFi.h>

#include "secrets.h"
#include "backend/TimeRetriever.h"
#include "backend/APICaller.h"
#include "backend/RouteRetriever.h"
#include "backend/StopRetriever.h"
#include "backend/DepartureRetriever.h"

const std::vector<std::string> whitelistV = {"o-9q9-bart", "o-9q9-caltrain"};
TimeRetriever timeR;
Whitelist whitelist(whitelistV, false);
APICaller apiCaller(Secrets::SECRET_API_KEY);
StopRetriever stopRetriever(&apiCaller, 37.6002, -122.3867, 100, whitelist);

void connectToWifi()
{
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(Secrets::SECRET_SSID);
  WiFi.begin(Secrets::SECRET_SSID, Secrets::SECRET_PASSWORD);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }
  Serial.print("Connected to ");
  Serial.println(Secrets::SECRET_SSID);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  connectToWifi();
  timeR.sync();

  RouteRetriever routeRetriever(&apiCaller, 37.79286775914171, -122.39696985281753, 100, whitelist);
  Stop stop{"s-9q8yyzcnrh-embarcadero", "EmbarcarderoN"};

  routeRetriever.retrieve();
  RouteList rl = routeRetriever.getRouteList();

  DepartureRetriever depRetriever(&apiCaller, &timeR, stop, rl, 5, 6000, 60);
  depRetriever.retrieve();
  DepartureList dl = depRetriever.getDepartureList();
  dl.debugPrintAllDepartures();
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(500);
}
