#include <WiFi.h>
#include <vector>
#include <time.h>

#include "RouteTable.h"
#include "StopTable.h"
#include "TransitZone.h"
#include "arduino_secrets.h"
#include "constants.h"
#include "Stop.h"
#include "Bridge.h"

RouteTable routeTable;
StopTable stopTable;
Bridge bridge;

time_t retrieveCurTime() {
  configTime(0, 0, TIME_URL);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Cannot get local time");
    return -1;
  }
  
  return Stop::timegm_custom(&timeinfo);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  // connect to Wifi
  delay(100);
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(SECRET_SSID);
  WiFi.begin(SECRET_SSID, SECRET_PASSWORD);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }
  Serial.print("Connected to ");
  Serial.println(SECRET_SSID);

  delay(3000);

  Serial.println("Pinging API...");
  time_t unixTime = retrieveCurTime();
  long long ms = millis();
  
  // initialize http
  // http.useHTTP10(true);

  // ----- TESTING ----
  std::vector<String> testFilter = {/*"o-9q5-metro~losangeles",*/ "o-9qh-metrolinktrains", "o-9q9-bart", "o-9q8y-sfmta"};

  // TransitZone zone("Westwood / Rancho Park", &routeTable, &stopTable, 34.036565, -118.424929, 100); // s-9q5c9hjyg6-westwood~ranchoparkstation
  // TransitZone zone("Westwood / Weyburn", &routeTable, &stopTable, 34.062591, -118.445390, 100); // s-9q5cb8yteq-westwood~weyburn
  // TransitZone zone("Embarcadero", &routeTable, &stopTable, 37.7928486,-122.3968361, 100); // s-9q8yyzcnrh-embarcadero
  TransitZone zone("Union Station", &routeTable, &stopTable, 34.055244, -118.233776, 200);
  zone.setWhiteList(&testFilter);
  // zone.init();
  // zone.updateDepartures(retrieveCurTime());
  // zone.debugPrint();

  bridge.setZone(&zone);
  bridge.setTimeSync(ms, unixTime);
  bridge.retrieveDepartures();

  bridge.debugPrintRoutes();
  bridge.debugPrintDepartures();

  // Stop wwrp("s-9q5cb8yteq-westwood~weyburn", "Westwood / Weyburn", "", NUM_ROUTES_STORED, &routeTable, &http);
  // wwrp.callDeparturesAPI();
  // wwrp.debugPrintStop();

  // Serial.println("Valid Stop: " + String(westwoodRancho.getIsValidStop()));
  // Serial.println("Stop Name: " + String(westwoodRancho.getName()));

  // for (int i = 0; i < routes->size(); i++) {
  //   Route *r = routes->at(i);
  //   printRoute(r);
  // }
  Serial.println("Finished");
}

void loop()
{
  // put your main code here, to run repeatedly:
}
