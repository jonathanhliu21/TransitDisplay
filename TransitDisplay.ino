#include <WiFi.h>
#include <HTTPClient.h>
#include <vector>

#include "RouteTable.h"
#include "StopTable.h"
#include "TransitZone.h"
#include "arduino_secrets.h"
#include "constants.h"
#include "Stop.h"

HTTPClient http;
RouteTable routeTable(&http);
StopTable stopTable;

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
  
  // initialize http
  // http.useHTTP10(true);

  // ----- TESTING ----
  std::vector<String> testFilter = {"o-9q5-metro~losangeles", "o-9qh-metrolinktrains", "o-9q9-bart", "o-9q8y-sfmta"};

  // TransitZone zone("Westwood / Rancho Park", &routeTable, &stopTable, &http, 34.036565, -118.424929, 100); // s-9q5c9hjyg6-westwood~ranchoparkstation
  TransitZone zone("Westwood / Weyburn", &routeTable, &stopTable, &http, 34.062591, -118.445390, 100); // s-9q5cb8yteq-westwood~weyburn
  // TransitZone zone("Embarcadero", &routeTable, &stopTable, &http, 37.7928486,-122.3968361, 100); // s-9q8yyzcnrh-embarcadero
  // TransitZone zone("Union Station", &routeTable, &stopTable, &http, 34.055244, -118.233776, 200);
  zone.setWhiteList(&testFilter);

  zone.init();
  zone.debugPrint();

  Stop wwrp("s-9q5cb8yteq-westwood~weyburn", "Westwood / Weyburn", "", NUM_ROUTES_STORED, &routeTable, &http);
  wwrp.callDeparturesAPI();
  wwrp.debugPrintStop();

  // Serial.println("Valid Stop: " + String(westwoodRancho.getIsValidStop()));
  // Serial.println("Stop Name: " + String(westwoodRancho.getName()));

  // for (int i = 0; i < routes->size(); i++) {
  //   Route *r = routes->at(i);
  //   printRoute(r);
  // }
}

void loop()
{
  // put your main code here, to run repeatedly:
}
