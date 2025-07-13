#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "RouteTable.h"
#include "Stop.h"
#include "arduino_secrets.h"
#include "constants.h"

WiFiClientSecure wifiClient;
HttpClient http(wifiClient, TRANSIT_LAND_SERVER, TRANSIT_LAND_PORT);
RouteTable routeTable;

void printRoute(const Route* route) {
  Serial.println(F("--- Route Info ---"));
  
  Serial.print(F("ID: "));
  Serial.println(route->id);

  Serial.print(F("Name: "));
  Serial.println(route->name);

  Serial.print(F("Line Color: 0x"));
  Serial.println(route->lineColor, HEX);

  Serial.print(F("Text Color: 0x"));
  Serial.println(route->textColor, HEX);

  Serial.print(F("Agency ID: "));
  Serial.println(route->agencyId);
  
  Serial.println(F("------------------"));
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

  // set certificate for https
  wifiClient.setCACert(TRANSIT_LAND_ROOT_CERTIFICATE);

  Stop westwoodRancho("s-9q5c9hjyg6-westwood~ranchoparkstation", NUM_ROUTES_STORED, &routeTable, &http);
  Serial.println("Initializing Westwood/Rancho Park...");
  westwoodRancho.init();

  Serial.println("Valid Stop: " + String(westwoodRancho.getIsValidStop()));
  Serial.println("Stop Name: " + String(westwoodRancho.getName()));
  Serial.println("Lat: " + String(westwoodRancho.getLat()));
  Serial.println("Lon: " + String(westwoodRancho.getLon()));

  auto routes = westwoodRancho.getRoutes();
  for (int i = 0; i < routes->size(); i++) {
    Route *r = routes->at(i);
    printRoute(r);
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
}
