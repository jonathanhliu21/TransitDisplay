#include <WiFi.h>
#include <vector>
#include <time.h>
#include <TFT_eSPI.h>

#include <algorithm>
#include <vector>
#include <tuple>

#include "ButtonReader.h"
#include "RouteTable.h"
#include "StopTable.h"
#include "TransitZone.h"
#include "arduino_secrets.h"
#include "constants.h"
#include "Stop.h"
#include "Bridge.h"
#include "Overpass_Regular12.h"
#include "Overpass_Regular16.h"
#include "DisplayConstants.h"
#include "UserConstants.h"

ButtonReader reader1(BUTTON_1_PIN, DEBOUNCE_DELAY);
ButtonReader reader2(BUTTON_2_PIN, DEBOUNCE_DELAY);

TFT_eSPI tft = TFT_eSPI();
RouteTable routeTable;
StopTable stopTable;
Bridge bridge(&tft);
unsigned long lastSyncTime;

// set globals from user constants
#ifdef TRANSIT_USE_FILTER
std::vector<String> transitFilter = TRANSIT_FILTER;
#endif
std::vector<TransitZone*> zones;

// set state stuff
int curIndex = 0;
enum class State {
  SELECT,
  ARE_YOU_SURE,
  TRANSIT,
};
State state = State::SELECT;

time_t retrieveCurTime() {
  configTime(0, 0, TIME_URL);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Cannot get local time");
    return -1;
  }
  
  return Stop::timegm_custom(&timeinfo);
}

void drawNoZonesFound() {
  // draw the title
  tft.fillScreen(TFT_BLACK);
  tft.loadFont(Overpass_Regular12); // Must match the .vlw file name
  tft.setTextSize(12);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("No zones found", DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2);
  tft.unloadFont();
}

void drawSelectScreen() {
  // draw the title
  tft.fillScreen(TFT_BLACK);
  tft.loadFont(Overpass_Regular16); // Must match the .vlw file name
  tft.setTextSize(16);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(TC_DATUM);
  tft.setTextWrap(false);
  tft.drawString(zones[curIndex]->getName(), DISPLAY_WIDTH / 2, NAME_Y);
  tft.unloadFont();

  // draw the coordinates
  String latStr(zones[curIndex]->getLat(), COORD_DIGITS);
  String lonStr(zones[curIndex]->getLon(), COORD_DIGITS);
  String radStr(zones[curIndex]->getRadius(), RAD_DIGITS);
  tft.loadFont(Overpass_Regular12); // Must match the .vlw file name
  tft.setTextSize(12);
  tft.setTextColor(TFT_DARKGREY);
  tft.drawString(latStr + ", " + lonStr, DISPLAY_WIDTH / 2, COORDS_Y);
  tft.drawString("Radius: " + radStr + "m", DISPLAY_WIDTH / 2, RADIUS_Y);

  // draw the filter
  tft.setTextWrap(true);
  tft.setCursor(FILTER_CURSOR_X, FILTER_CURSOR_Y);
  tft.setTextColor(TFT_WHITE);
  String concatResult = "Onestop ID Filter: ";
#ifdef TRANSIT_USE_FILTER
  for (const auto &str : transitFilter) {
    concatResult += str + ", ";
  }
  concatResult.remove(concatResult.length() - 2);
#else
  concatResult += "No filter in use";
#endif
  tft.print(concatResult);

  // draw the controls
  tft.setTextDatum(TC_DATUM);
  tft.fillRect(0, SELECT_INSTRUCTION_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT - SELECT_INSTRUCTION_Y, TFT_BLACK);
  if (zones.size() > 1) {
    int nextIndex = curIndex+1;
    if (nextIndex >= zones.size()) nextIndex = 0;
    tft.setTextColor(TFT_GREEN);
    tft.drawString("1 - Start", DISPLAY_WIDTH / 2 - START_INSTRUCTION_X_OFFSET, SELECT_INSTRUCTION_Y);
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(TL_DATUM);

    // 1. Define the string components and its starting position
    String prefix = "2 - Next (";
    String zoneName = zones[nextIndex]->getName();
    String fullText = prefix + zoneName + ")";
    int startX = DISPLAY_WIDTH / 2 + NEXT_INSTRUCTION_X_OFFSET;

    // 2. Check if the full, untruncated string will fit on the screen
    if (startX + tft.textWidth(fullText) <= DISPLAY_WIDTH) {
      // If it fits, draw the original string
      tft.drawString(fullText, startX, SELECT_INSTRUCTION_Y);
    } else {
      // If it doesn't fit, we must truncate it
      String ellipsisSuffix = "...)" ;
      
      // Calculate the maximum pixel width available for the zone name itself
      int maxNameWidth = DISPLAY_WIDTH - startX - tft.textWidth(prefix) - tft.textWidth(ellipsisSuffix);

      // Shorten the zone name until it fits within the maxNameWidth
      String truncatedName = zoneName;
      while (tft.textWidth(truncatedName) > maxNameWidth && truncatedName.length() > 0) {
        truncatedName.remove(truncatedName.length() - 1); // Remove the last character
      }

      // 3. Construct and draw the final, truncated string
      String displayText = prefix + truncatedName + ellipsisSuffix;
      tft.drawString(displayText, startX, SELECT_INSTRUCTION_Y);
    }
  } else {
    tft.setTextColor(TFT_GREEN);
    tft.drawString("1 - Start", DISPLAY_WIDTH / 2, SELECT_INSTRUCTION_Y);
  }
 
  tft.unloadFont();
}

void fillZones() {
  using ZoneElement = std::tuple<String, float, float, float>;
  std::vector<ZoneElement> zoneNames = TRANSIT_ZONES;
  for (int i = 0; i < zoneNames.size(); i++) {
    String name = std::get<0>(zoneNames[i]);
    float lat = std::get<1>(zoneNames[i]);
    float lon = std::get<2>(zoneNames[i]);
    float radius = std::get<3>(zoneNames[i]);
    TransitZone *zone = new TransitZone(name, &routeTable, &stopTable, lat, lon, radius);
#ifdef TRANSIT_USE_FILTER
    zone->setWhiteList(&transitFilter);
#endif
    zones.push_back(zone);
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(ROUTE_PIN, OUTPUT);
  pinMode(STOP_PIN, OUTPUT);
  pinMode(DEP_PIN, OUTPUT);
  pinMode(RATE_LIMIT_PIN, OUTPUT);

  pinMode(BUTTON_1_PIN, INPUT);
  pinMode(BUTTON_2_PIN, INPUT);

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

  fillZones();
  if (zones.size() > 0) {
    drawSelectScreen();
  } else {
    drawNoZonesFound();
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (zones.size() == 0) return;

  bool button1Res = reader1.readButton();
  bool button2Res = reader2.readButton();
  unsigned long curMs = millis();

  switch (state) {
    case State::SELECT:
      digitalWrite(ROUTE_PIN, LOW);
      digitalWrite(STOP_PIN, LOW);
      digitalWrite(DEP_PIN, LOW);
      digitalWrite(RATE_LIMIT_PIN, LOW);
      if (button1Res) {
        bridge.setZone(zones[curIndex], curMs, retrieveCurTime());
        state = State::TRANSIT;
      }
      if (button2Res && zones.size() > 1) {
        curIndex++;
        if (curIndex >= zones.size()) curIndex = 0;
        drawSelectScreen();
      }
      break;
    case State::ARE_YOU_SURE:
      if (button1Res) {
        drawSelectScreen();
        state = State::SELECT;
      }
      if (button2Res) {
        bridge.drawTitle();
        bridge.syncTime(curMs, retrieveCurTime());
        state = State::TRANSIT;
      }
      break;
    case State::TRANSIT:
      if (button1Res || button2Res) {
        bridge.drawAreYouSure();
        state = State::ARE_YOU_SURE;
      }
      bridge.loop();
      break;
  }  
  
  if (curMs - lastSyncTime > SYNC_TIME_FREQ) {
    bridge.syncTime(curMs, retrieveCurTime());
    lastSyncTime = curMs;
  }
}
