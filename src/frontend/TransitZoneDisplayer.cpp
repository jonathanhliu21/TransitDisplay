#include "frontend/TransitZoneDisplayer.h"

#include "Constants.h"

namespace
{
  const int START_INSTRUCTION_X_OFFSET = 50;
  const int NEXT_INSTRUCTION_X_OFFSET = 20;
  const int SELECT_INSTRUCTION_Y = 285;
}

TransitZoneDisplayer::TransitZoneDisplayer(const std::string &name,
                                           TFT_eSPI *tft,
                                           const uint8_t *fontRegular,
                                           const uint8_t *fontLarge,
                                           int routeRefreshPeriod,
                                           int departuresRefreshPeriod)
    : m_name{name},
      m_tft{tft},
      m_fontRegular{fontRegular},
      m_fontLarge{fontLarge},
      m_routeRefreshPeriod{routeRefreshPeriod},
      m_departuresRefreshPeriod{departuresRefreshPeriod},
      m_lastRouteRefresh{0},
      m_lastDeparturesRefresh{0},
      m_routeDisplay{tft, fontRegular},
      m_departuresDisplay{tft, fontRegular}
{
}

void TransitZoneDisplayer::setRoutes(const std::vector<DisplayRoute> &displayRoutes)
{
  m_routeDisplay.setRoutes(displayRoutes);
}

void TransitZoneDisplayer::setDepartures(const std::vector<DisplayDeparture> &displayDeps)
{
  m_departuresDisplay.setDepartures(displayDeps);
}

void TransitZoneDisplayer::drawInitializing()
{
  m_tft->fillScreen(TFT_BLACK);
  m_tft->loadFont(m_fontRegular); // Must match the .vlw file name
  m_tft->setTextColor(TFT_WHITE);
  m_tft->setTextDatum(MC_DATUM);
  m_tft->drawString("Initializing...",
                    Constants::DISPLAY_WIDTH / 2,
                    Constants::DISPLAY_HEIGHT / 2);
  m_tft->unloadFont();
}

void TransitZoneDisplayer::drawAreYouSure()
{
  // draw the "are you sure?"
  m_tft->fillScreen(TFT_BLACK);
  m_tft->loadFont(m_fontRegular); // Must match the .vlw file name
  m_tft->setTextSize(12);
  m_tft->setTextColor(TFT_WHITE);
  m_tft->setTextDatum(MC_DATUM);
  m_tft->drawString(
      "Go back to selection page?",
      Constants::DISPLAY_WIDTH / 2,
      Constants::DISPLAY_HEIGHT / 2);

  // draw the controls
  m_tft->setTextDatum(TC_DATUM);
  m_tft->fillRect(
      0,
      SELECT_INSTRUCTION_Y,
      Constants::DISPLAY_WIDTH,
      Constants::DISPLAY_HEIGHT - SELECT_INSTRUCTION_Y,
      TFT_BLACK);
  m_tft->setTextColor(TFT_WHITE);
  m_tft->drawString("1 - Yes",
                    Constants::DISPLAY_WIDTH / 2 - START_INSTRUCTION_X_OFFSET,
                    SELECT_INSTRUCTION_Y);
  m_tft->setTextDatum(TL_DATUM);
  m_tft->setTextColor(TFT_DARKGREY);
  m_tft->drawString("2 - No",
                    Constants::DISPLAY_WIDTH / 2 + NEXT_INSTRUCTION_X_OFFSET,
                    SELECT_INSTRUCTION_Y);
  m_tft->unloadFont();
}

void TransitZoneDisplayer::cycle()
{
  // capture timestamp of BEGINNING of cycle
  m_lastRouteRefresh = millis();
  m_routeDisplay.cycle();

  m_lastDeparturesRefresh = millis();
  m_departuresDisplay.cycle();
}

void TransitZoneDisplayer::loop()
{
  // check route display; BEGINNING of cycle
  std::time_t curTime = millis();
  if (curTime - m_lastRouteRefresh >= m_routeRefreshPeriod)
  {
    m_routeDisplay.cycle();
    m_lastRouteRefresh = curTime;
  }

  // check departure display; BEGINNING of cycle
  curTime = millis();
  if (curTime - m_lastDeparturesRefresh >= m_routeRefreshPeriod)
  {
    m_routeDisplay.cycle();
    m_lastRouteRefresh = curTime;
  }
}