#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>

#include "secrets.h"
#include "types/Whitelist.h"
#include "backend/TimeRetriever.h"
#include "backend/APICaller.h"
#include "backend/TransitZone.h"
#include "frontend/TransitZoneDisplayer.h"
#include "fonts/Overpass_Regular12.h"
#include "fonts/Overpass_Regular16.h"
#include "ZoneManager.h"

const std::vector<std::string> whitelistV = {
    "o-9q9-bart",
    "o-9q9-caltrain", "o-9q5-metro~losangeles", "o-9q5c-bigbluebus", "o-dr5r-nyct"};
TimeRetriever timeR;
Whitelist whitelist(whitelistV);
APICaller apiCaller(Secrets::SECRET_API_KEY);
TFT_eSPI tft;
TransitZoneDisplayer displayer{
    "Test",
    &tft,
    Overpass_Regular12,
    Overpass_Regular16,
    5000,
    10000};

TransitZone zone{"WWRP", 34.03681632305407, -118.42457036391623, 100, &apiCaller, &timeR, {5, 6000, 60}};
ZoneManager *manager;

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
  tft.begin();
  tft.setRotation(1); // Depending of the use-case.

  connectToWifi();
  timeR.sync();

  manager = new ZoneManager(&zone, &tft, &timeR, whitelist, Overpass_Regular12, Overpass_Regular16);
  manager->init();
}

void loop()
{
  manager->mainThreadLoop();
  delay(10);
}
