#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoHttpClient.h>
#include <vector>

#include "RouteTable.h"
#include "Stop.h"
#include "arduino_secrets.h"
#include "constants.h"

WiFiClientSecure wifiClient;
HttpClient http(wifiClient, TRANSIT_LAND_SERVER, TRANSIT_LAND_PORT);
RouteTable routeTable(&http);

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

  // set certificate for https
  wifiClient.setCACert(TRANSIT_LAND_ROOT_CERTIFICATE);

  // ----- TESTING ----
  // 34.036565, -118.424929 (WW/RP all stops)
  // 34.036816, -118.424576 (WW/RP E line)
  // 34.062591, -118.445390 (WW/WB all)
  // 37.7928486,-122.3968361 (EMBR)
  // routeTable.retrieveRoutes(34.036565, -118.424929, SEARCH_RADIUS);
  routeTable.retrieveRoutes(37.7928486,-122.3968361, SEARCH_RADIUS);
  routeTable.debugPrintAllRoutes();

  // Stop westwoodRancho("s-9q5c9hjyg6-westwood~ranchoparkstation", NUM_ROUTES_STORED, &routeTable, &http);
  // Serial.println("Initializing Westwood/Rancho Park...");
  // westwoodRancho.init();

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
