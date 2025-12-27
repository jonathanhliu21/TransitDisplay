#include "frontend/RouteDisplayer.h"

#include <Arduino.h>
#include <vector>

#include "types/DisplayTypes.h"
#include "Constants.h"

namespace
{
  const int ROUTE_START_Y = 55;
  const int ROUTE_BUTTON_HEIGHT = 30;
  const int ROUTE_PADDING = Constants::DISPLAY_ROUTE_PADDING;
  const int ROUTE_TEXT_Y_OFFSET = 5;
  const int ROUTE_GAP = 6;
}

RouteDisplayer::RouteDisplayer(TFT_eSPI *tft, const uint8_t *fontRegular) : m_tft{tft}, m_curStartPtr{0}, m_fontRegular{fontRegular} {}

void RouteDisplayer::setRoutes(std::vector<DisplayRoute> routes)
{
  m_displayRoutes = routes;
  m_curStartPtr = 0;
}

void RouteDisplayer::cycle()
{
  m_tft->setTextDatum(MC_DATUM);
  m_tft->loadFont(m_fontRegular); // Must match the .vlw file name

  int startPtr = m_curStartPtr;
  if (startPtr >= m_displayRoutes.size())
    startPtr = 0;
  int totalWidth = preCalculation();
  // int totalWidth = 0;

  // =======================================================
  // 2. DRAWING PASS: Use the total width to center and draw
  // =======================================================
  int startX = (Constants::DISPLAY_WIDTH - totalWidth) / 2; // Calculate centered start X
  int currentX = startX + ROUTE_GAP;

  // Clear screen only if we have another page
  if (!(startPtr == 0 && m_curStartPtr == m_displayRoutes.size()))
  {
    m_tft->fillRect(0, ROUTE_START_Y, Constants::DISPLAY_WIDTH, ROUTE_BUTTON_HEIGHT, TFT_BLACK);
  }

  for (int i = startPtr; i < m_curStartPtr; i++)
  {
    int textW = m_tft->textWidth(m_displayRoutes[i].name.c_str());
    int buttonWidth = textW + ROUTE_PADDING;

    uint16_t lineColor = hexToRGB565(m_displayRoutes[i].lineColor);
    uint16_t textColor = hexToRGB565(m_displayRoutes[i].textColor);

    m_tft->fillRoundRect(currentX, ROUTE_START_Y, buttonWidth, ROUTE_BUTTON_HEIGHT, 4, lineColor);
    m_tft->setTextColor(textColor);
    m_tft->drawString(m_displayRoutes[i].name.c_str(), currentX + buttonWidth / 2, ROUTE_START_Y + ROUTE_BUTTON_HEIGHT / 2 + ROUTE_TEXT_Y_OFFSET);

    currentX += buttonWidth + ROUTE_GAP;
  }
  m_tft->unloadFont();
}

// returns total width
int RouteDisplayer::preCalculation()
{
  // =======================================================
  // 1. PRE-CALCULATION PASS: Measure the total width first
  // =======================================================
  if (m_displayRoutes.size() == 0)
    return 0;

  if (m_curStartPtr >= m_displayRoutes.size())
  {
    m_curStartPtr = 0;
  }

  int totalWidth = ROUTE_GAP;
  std::string curAgency = m_displayRoutes[m_curStartPtr].agencyOnestopId;
  int prevStartPtr = m_curStartPtr;
  m_curStartPtr = m_displayRoutes.size();
  for (int i = prevStartPtr; i < m_displayRoutes.size(); i++)
  {
    int textW = m_tft->textWidth(m_displayRoutes[i].name.c_str());
    if (totalWidth + textW + ROUTE_PADDING + ROUTE_GAP > Constants::DISPLAY_WIDTH || m_displayRoutes[i].agencyOnestopId != curAgency)
    {
      // cut off if doesn't fit on screen or the agency ID doesn't match
      m_curStartPtr = i;
      break;
    }

    totalWidth += textW + ROUTE_PADDING; // Add button width
    // Serial.print("text w: ");
    // Serial.println(tft.textWidth(m_displayRoutes[i].name));
    totalWidth += ROUTE_GAP;
  }

  // if one long ass route don't render it
  if (m_curStartPtr == prevStartPtr && m_curStartPtr < m_displayRoutes.size())
  {
    m_curStartPtr++;
    return m_tft->textWidth(m_displayRoutes[prevStartPtr].name.c_str());
  }

  // Serial.print("start ptr: ");
  // Serial.println(m_curStartPtr);

  return totalWidth;
}