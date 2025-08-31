#include <WiFi.h>
#include <vector>
#include <time.h>
#include <TFT_eSPI.h>

#include "RouteTable.h"
#include "StopTable.h"
#include "TransitZone.h"
#include "arduino_secrets.h"
#include "constants.h"
#include "Stop.h"
#include "Bridge.h"
#include "Overpass_Regular12.h"
#include "DisplayConstants.h"
#include "UserConstants.h"

TFT_eSPI tft = TFT_eSPI();
RouteTable routeTable;
StopTable stopTable;
Bridge bridge(&tft);
unsigned long lastSyncTime;

// set globals from user constants
#ifdef TRANSIT_USE_FILTER
std::vector<String> testFilter = TRANSIT_FILTER;
#endif
TransitZone zone(TRANSIT_ZONE_NAME, &routeTable, &stopTable, TRANSIT_PIN_LAT, TRANSIT_PIN_LON, TRANSIT_PIN_RADIUS);

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

  pinMode(ROUTE_PIN, OUTPUT);
  pinMode(STOP_PIN, OUTPUT);
  pinMode(DEP_PIN, OUTPUT);
  pinMode(RATE_LIMIT_PIN, OUTPUT);

  digitalWrite(ROUTE_PIN, LOW);
  digitalWrite(STOP_PIN, LOW);
  digitalWrite(DEP_PIN, LOW);
  digitalWrite(RATE_LIMIT_PIN, LOW);

  tft.begin();
  tft.setRotation(1); // Depending of the use-case.

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

  Serial.println("Pinging API...");
  tft.fillScreen(TFT_BLACK);
  tft.loadFont(Overpass_Regular12); // Must match the .vlw file name
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Initializing...", DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2);
  tft.unloadFont();

  time_t unixTime = retrieveCurTime();
  long long ms = millis();
  lastSyncTime = ms;

#ifdef TRANSIT_USE_FILTER
  zone.setWhiteList(&testFilter);
#endif

  bridge.setZone(&zone, ms, unixTime);

  Serial.println("Finished");
}

void loop()
{
  // put your main code here, to run repeatedly:
  bridge.loop();
  
  unsigned long curMs = millis();
  if (curMs - lastSyncTime > SYNC_TIME_FREQ) {
    bridge.syncTime(curMs, retrieveCurTime());
    lastSyncTime = curMs;
  }
}
