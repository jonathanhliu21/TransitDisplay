#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>

#include "Configuration.h"
#include "Constants.h"
#include "ZoneManager.h"
#include "ButtonReader.h"

enum class State
{
  SELECT,
  ARE_YOU_SURE,
  TRANSIT
};

Configuration config;
ZoneManager *zoneManager;
int zoneIdx = 0;
State state(State::SELECT);
ZoneListDisplayer *displayer;
TFT_eSPI *tft;
TimeRetriever *timeRetriever;
Whitelist whitelist;

ButtonReader reader1(Constants::BUTTON_1_PIN);
ButtonReader reader2(Constants::BUTTON_2_PIN);

std::vector<TransitZone *> zones;

void configurePins()
{
  pinMode(Constants::ROUTE_ERROR_PIN, OUTPUT);
  pinMode(Constants::STOP_ERROR_PIN, OUTPUT);
  pinMode(Constants::DEPARTURE_ERROR_PIN, OUTPUT);
  pinMode(Constants::RATE_LIMIT_PIN, OUTPUT);

  pinMode(Constants::BUTTON_1_PIN, INPUT);
  pinMode(Constants::BUTTON_2_PIN, INPUT);

  digitalWrite(Constants::ROUTE_ERROR_PIN, LOW);
  digitalWrite(Constants::STOP_ERROR_PIN, LOW);
  digitalWrite(Constants::DEPARTURE_ERROR_PIN, LOW);
  digitalWrite(Constants::RATE_LIMIT_PIN, LOW);
}

void connectToWifi()
{
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(config.getSSID().c_str());
  WiFi.begin(config.getSSID().c_str(), config.getWifiPassword().c_str());

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }
  Serial.print("Connected to ");
  Serial.println(config.getSSID().c_str());
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  // configure globals
  config.init();
  zones = config.getZones();
  displayer = config.getZoneListDisplayer();
  tft = config.getTFT();
  timeRetriever = config.getTimeRetriever();
  whitelist = config.getWhitelist();

  // configure pins
  configurePins();

  // configure tft
  tft->begin();
  tft->setRotation(1); // Depending on the use-case.

  // connect to wifi
  displayer->drawConnecting();
  connectToWifi();

  // sync time
  if (!timeRetriever->sync())
  {
    digitalWrite(Constants::ROUTE_ERROR_PIN, HIGH);
    digitalWrite(Constants::STOP_ERROR_PIN, HIGH);
    digitalWrite(Constants::DEPARTURE_ERROR_PIN, HIGH);
    digitalWrite(Constants::RATE_LIMIT_PIN, HIGH);
    return;
  }

  // draw screen
  if (zones.empty())
  {
    displayer->drawNoZonesFound();
  }
  else if (zones.size() == 1)
  {
    displayer->drawZone(zones[0], nullptr, whitelist);
  }
  else
  {
    displayer->drawZone(zones[0], zones[1], whitelist);
  }
}

void drawSelectScreen()
{
  int nextZoneIdx = zoneIdx + 1;
  if (nextZoneIdx >= zones.size())
    nextZoneIdx = 0;
  displayer->drawZone(zones[zoneIdx], zones[nextZoneIdx], whitelist);
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (zones.empty())
    return;

  bool button1Res = reader1.readButton();
  bool button2Res = reader2.readButton();
  unsigned long curMs = millis();

  switch (state)
  {
  case State::SELECT:
    digitalWrite(Constants::ROUTE_ERROR_PIN, LOW);
    digitalWrite(Constants::STOP_ERROR_PIN, LOW);
    digitalWrite(Constants::DEPARTURE_ERROR_PIN, LOW);
    digitalWrite(Constants::RATE_LIMIT_PIN, LOW);
    if (button1Res)
    {
      zoneManager = new ZoneManager(zones[zoneIdx], tft, timeRetriever, whitelist, config.getRegularFont(), config.getTitleFont());
      state = State::TRANSIT;
      zoneManager->init();
    }
    else if (button2Res && zones.size() > 1)
    {
      zoneIdx++;
      if (zoneIdx >= zones.size())
        zoneIdx = 0;

      drawSelectScreen();
    }
    break;
  case State::ARE_YOU_SURE:
    if (button1Res)
    {
      state = State::SELECT;
      delete zoneManager;
      drawSelectScreen();
    }
    if (button2Res)
    {
      zoneManager->cycleDisplay();
      state = State::TRANSIT;
    }
    break;
  case State::TRANSIT:
    if (button1Res || button2Res)
    {
      zoneManager->drawAreYouSure();
      state = State::ARE_YOU_SURE;
    }
    else
    {
      zoneManager->mainThreadLoop();
    }
    delay(10);
  default:
    break;
  }
}
