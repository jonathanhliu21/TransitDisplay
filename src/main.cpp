#include <Arduino.h>
#include <WiFi.h>

#include "secrets.h"
#include "backend/TimeRetriever.h"
#include "backend/APICaller.h"
#include "backend/TransitZone.h"

const std::vector<std::string> whitelistV = {
    "o-9q9-bart",
    "o-9q9-caltrain", "o-9q5-metro~losangeles", "o-9q5c-bigbluebus", "o-dr5r-nyct"};
TimeRetriever timeR;
Whitelist whitelist(whitelistV);
APICaller apiCaller(Secrets::SECRET_API_KEY);

TransitZone zone{"WWRP", 34.03681632305407, -118.42457036391623, 100, &apiCaller, &timeR, {5, 6000, 60}};

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

  zone.init(whitelist);
  zone.callDeparturesAPI();
  zone.debugPrint();
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(500);
}
