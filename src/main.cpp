#include <Arduino.h>
#include <WiFi.h>

#include "secrets.h"
#include "backend/APICaller.h"
#include "backend/RouteRetriever.h"
#include "backend/StopRetriever.h"

const std::vector<std::string> whitelistV = {"o-9q9-bart", "o-9q9-caltrain"};
Whitelist whitelist(whitelistV, false);
APICaller apiCaller(Secrets::SECRET_API_KEY);
RouteRetriever routeRetriever(&apiCaller, 37.6002, -122.3867, 100, whitelist);
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

  stopRetriever.retrieve();                   // call API
  StopList res = stopRetriever.getStopList(); // get routelist object
  res.debugPrintAllStops();                   // print
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(500);
}
