#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>

#include "secrets.h"
#include "backend/TimeRetriever.h"
#include "backend/APICaller.h"
#include "backend/TransitZone.h"
#include "frontend/TransitZoneDisplayer.h"
#include "fonts/Overpass_Regular12.h"
#include "fonts/Overpass_Regular16.h"

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

  zone.init(whitelist);
  zone.callDeparturesAPI();
  zone.debugPrint();

  std::vector<DisplayRoute> routes{
      {"", "1X", 0xFFFFFF, 0, "a"},
      {"", "1Xadskljfklasdfjlasdkfklasdjfkl", 0xFFFFFF, 0, "a"},
      {"", "X", 0xFFFFFF, 0, "a"},
      {"", "1", 0xFFFFFF, 0, "a"},
      {"", "1Xdf", 0xFFFFFF, 0, "a"},
      {"", "asdf", 0xFFFFFF, 0, "a"},
      {"", "t", 0xFFFFFF, 0, "a"},
      {"", "efasd", 0xFFFFFF, 0, "a"},
      {"", "n", 0xFFFFFF, 0, "a"},
      {"", "a", 0xFFFFFF, 0, "a"},
      {"", "b", 0xFFFFFF, 0, "a"},
      {"", "2", 0xFFFFFF, 0, "b"},
  };

  std::vector<DisplayDeparture> departures{
      {"abc", "123", 50, 0, 0xFFFFFF, 0xFFFFFF},
      {"asdjklfasdjlfkalsfklasdfjklasdjklflasdfdklasfladsfjlj", "1Xadsklfadskl", 20, 0, 0xFFFFFF, 0xFFFFFF},
      {"asdjklfasdjlfkalsfkl", "123", 30, 0, 0xFFFFFF, 0xFFFFFF},
      {"a", "1Xadsklfadskl", 40, 0, 0xFFFFFF, 0xFFFFFF},
      {"abc2", "123", 50, 0, 0xFFFFFF, 0xFFFFFF},
      {"abc3", "123", 50, 0, 0xFFFFFF, 0xFFFFFF},
      {"abc4", "123", 50, 0, 0xFFFFFF, 0xFFFFFF},
      {"abc5", "123", 50, 0, 0xFFFFFF, 0xFFFFFF},
      {"abc6", "123", 50, 0, 0xFFFFFF, 0xFFFFFF},
  };

  displayer.setRoutes(routes);
  displayer.setDepartures(departures);

  displayer.drawInitializing();
  delay(1000);
  displayer.drawAreYouSure();
  delay(1000);
  displayer.cycle();
}

void loop()
{
  displayer.loop();
}
