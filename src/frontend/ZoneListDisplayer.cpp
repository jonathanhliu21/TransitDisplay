#include "frontend/ZoneListDisplayer.h"

#include <Arduino.h>
#include "Constants.h"

namespace
{
  const int START_INSTRUCTION_X_OFFSET = Constants::DISPLAY_START_INSTRUCTION_X_OFFSET;
  const int NEXT_INSTRUCTION_X_OFFSET = Constants::DISPLAY_NEXT_INSTRUCTION_X_OFFSET;
  const int SELECT_INSTRUCTION_Y = Constants::DISPLAY_SELECT_INSTRUCTION_Y;

  // for title
  const int NAME_X = Constants::DISPLAY_TITLE_X;
  const int NAME_Y = Constants::DISPLAY_TITLE_Y;

  const int COORD_DIGITS = 4;
  const int RAD_DIGITS = 0;
  const int COORDS_Y = 52;
  const int RADIUS_Y = 77;
  const int FILTER_CURSOR_X = 5;
  const int FILTER_CURSOR_Y = 110;
}

ZoneListDisplayer::ZoneListDisplayer(TFT_eSPI *tft,
                                     const uint8_t *fontRegular,
                                     const uint8_t *fontLarge) : m_tft{tft}, m_fontRegular{fontRegular}, m_fontLarge{fontLarge}
{
}

void ZoneListDisplayer::drawConnecting()
{
  m_tft->fillScreen(TFT_BLACK);
  m_tft->loadFont(m_fontRegular); // Must match the .vlw file name
  m_tft->setTextSize(12);
  m_tft->setTextColor(TFT_WHITE);
  m_tft->setTextDatum(MC_DATUM);
  m_tft->drawString("Connecting to WiFi...", Constants::DISPLAY_WIDTH / 2, Constants::DISPLAY_HEIGHT / 2);
  m_tft->unloadFont();
}

void ZoneListDisplayer::drawNoZonesFound()
{
  m_tft->fillScreen(TFT_BLACK);
  m_tft->loadFont(m_fontRegular); // Must match the .vlw file name
  m_tft->setTextSize(12);
  m_tft->setTextColor(TFT_WHITE);
  m_tft->setTextDatum(MC_DATUM);
  m_tft->drawString("No zones found", Constants::DISPLAY_WIDTH / 2, Constants::DISPLAY_HEIGHT / 2);
  m_tft->unloadFont();
}

void ZoneListDisplayer::drawZone(TransitZone *zone, TransitZone *next, const Whitelist &wl)
{
  // draw the title
  m_tft->fillScreen(TFT_BLACK);
  m_tft->loadFont(m_fontLarge); // Must match the .vlw file name
  m_tft->setTextSize(16);
  m_tft->setTextColor(TFT_WHITE);
  m_tft->setTextDatum(TC_DATUM);
  m_tft->setTextWrap(false);
  m_tft->drawString(zone->getName().c_str(), Constants::DISPLAY_WIDTH / 2, NAME_Y);
  m_tft->unloadFont();

  // draw the coordinates
  String latStr(zone->getLat(), COORD_DIGITS);
  String lonStr(zone->getLon(), COORD_DIGITS);
  String radStr(zone->getRadius(), RAD_DIGITS);
  m_tft->loadFont(m_fontRegular); // Must match the .vlw file name
  m_tft->setTextSize(12);
  m_tft->setTextColor(TFT_DARKGREY);
  m_tft->drawString(latStr + ", " + lonStr, Constants::DISPLAY_WIDTH / 2, COORDS_Y);
  m_tft->drawString("Radius: " + radStr + "m", Constants::DISPLAY_WIDTH / 2, RADIUS_Y);

  // draw the filter
  m_tft->setTextWrap(true);
  m_tft->setCursor(FILTER_CURSOR_X, FILTER_CURSOR_Y);
  m_tft->setTextColor(TFT_WHITE);
  String concatResult = "Onestop ID Filter: ";
  if (wl.isActive())
  {
    concatResult += wl.getWhiteListStr().c_str();
    concatResult.remove(concatResult.length() - 2);
  }
  else
  {
    concatResult += "No filter in use";
  }
  m_tft->print(concatResult);

  // draw the controls
  m_tft->setTextDatum(TC_DATUM);
  m_tft->fillRect(0, SELECT_INSTRUCTION_Y, Constants::DISPLAY_WIDTH, Constants::DISPLAY_HEIGHT - SELECT_INSTRUCTION_Y, TFT_BLACK);
  if (next != nullptr)
  {
    m_tft->setTextColor(TFT_GREEN);
    m_tft->drawString("1 - Start", Constants::DISPLAY_WIDTH / 2 - START_INSTRUCTION_X_OFFSET, SELECT_INSTRUCTION_Y);
    m_tft->setTextColor(TFT_WHITE);
    m_tft->setTextDatum(TL_DATUM);

    // 1. Define the string components and its starting position
    String prefix = "2 - Next (";
    String zoneName = next->getName().c_str();
    String fullText = prefix + zoneName + ")";
    int startX = Constants::DISPLAY_WIDTH / 2 + NEXT_INSTRUCTION_X_OFFSET;

    // 2. Check if the full, untruncated string will fit on the screen
    if (startX + m_tft->textWidth(fullText) <= Constants::DISPLAY_WIDTH)
    {
      // If it fits, draw the original string
      m_tft->drawString(fullText, startX, SELECT_INSTRUCTION_Y);
    }
    else
    {
      m_tft->setTextWrap(false);
      // If it doesn't fit, we must truncate it
      String ellipsisSuffix = "...)";

      // Calculate the maximum pixel width available for the zone name itself
      int maxNameWidth = Constants::DISPLAY_WIDTH - startX - m_tft->textWidth(prefix) - m_tft->textWidth(ellipsisSuffix);

      // Shorten the zone name until it fits within the maxNameWidth
      String truncatedName = zoneName;
      while (m_tft->textWidth(truncatedName) > maxNameWidth && truncatedName.length() > 0)
      {
        truncatedName.remove(truncatedName.length() - 1); // Remove the last character
      }

      // 3. Construct and draw the final, truncated string
      String displayText = prefix + truncatedName + ellipsisSuffix;
      m_tft->drawString(displayText, startX, SELECT_INSTRUCTION_Y);
      m_tft->setTextWrap(true);
    }
  }
  else
  {
    m_tft->setTextColor(TFT_GREEN);
    m_tft->drawString("1 - Start", Constants::DISPLAY_WIDTH / 2, SELECT_INSTRUCTION_Y);
  }

  m_tft->unloadFont();
}