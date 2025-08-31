#include "RouteDisplay.h"

#include <TFT_eSPI.h>
#include "DisplayConstants.h"
#include "RouteTable.h"
#include "Overpass_Regular12.h"

RouteDisplay::RouteDisplay(TFT_eSPI *tft) : m_tft{tft}, m_curStartPtr{0} {}

void RouteDisplay::setRoutes(std::vector<Route> routes) {
  m_routes = routes;
  m_curStartPtr = 0;
}

void RouteDisplay::cycle() {
  m_tft->setTextDatum(MC_DATUM);
  m_tft->loadFont(Overpass_Regular12); // Must match the .vlw file name

  int startPtr = m_curStartPtr;
  if (startPtr >= m_routes.size()) startPtr = 0;
  int totalWidth = preCalculation();
  // int totalWidth = 0;

  // =======================================================
  // 2. DRAWING PASS: Use the total width to center and draw
  // =======================================================
  int startX = (DISPLAY_WIDTH - totalWidth) / 2; // Calculate centered start X
  int currentX = startX + ROUTE_GAP;

  // Clear screen only if we have another page
  if (!(startPtr == 0 && m_curStartPtr == m_routes.size())) {
    m_tft->fillRect(0, ROUTE_START_Y, DISPLAY_WIDTH, ROUTE_BUTTON_HEIGHT, TFT_BLACK);
  }

  for (int i = startPtr; i < m_curStartPtr; i++) {
    int textW = m_tft->textWidth(m_routes[i].name);
    int buttonWidth = textW + ROUTE_PADDING;

    uint16_t lineColor = hexToRGB565(m_routes[i].lineColor);
    uint16_t textColor = hexToRGB565(m_routes[i].textColor);

    m_tft->fillRoundRect(currentX, ROUTE_START_Y, buttonWidth, ROUTE_BUTTON_HEIGHT, 4, lineColor);
    m_tft->setTextColor(textColor);
    m_tft->drawString(m_routes[i].name, currentX + buttonWidth / 2, ROUTE_START_Y + ROUTE_BUTTON_HEIGHT / 2 + ROUTE_TEXT_Y_OFFSET);

    currentX += buttonWidth + ROUTE_GAP;
  }
  m_tft->unloadFont();
}

// returns total width
int RouteDisplay::preCalculation() {
  // =======================================================
  // 1. PRE-CALCULATION PASS: Measure the total width first
  // =======================================================
  if (m_routes.size() == 0) return 0;

  if (m_curStartPtr >= m_routes.size()) {
    m_curStartPtr = 0;
  }

  int totalWidth = ROUTE_GAP;
  String curAgency = m_routes[m_curStartPtr].agencyId;
  int prevStartPtr = m_curStartPtr;
  m_curStartPtr = m_routes.size();
  for (int i = prevStartPtr; i < m_routes.size(); i++) {
    int textW = m_tft->textWidth(m_routes[i].name);
    if (totalWidth + textW + ROUTE_PADDING + ROUTE_GAP > DISPLAY_WIDTH || m_routes[i].agencyId != curAgency) {
      // cut off if doesn't fit on screen or the agency ID doesn't match
      m_curStartPtr = i;
      break;
    }

    totalWidth += textW + ROUTE_PADDING; // Add button width
    // Serial.print("text w: ");
    // Serial.println(tft.textWidth(m_routes[i].name));
    totalWidth += ROUTE_GAP;
  }

  // if one long ass route don't render it
  if (m_curStartPtr == prevStartPtr && m_curStartPtr < m_routes.size()) {
    m_curStartPtr++;
    return m_tft->textWidth(m_routes[prevStartPtr].name);
  }

  // Serial.print("start ptr: ");
  // Serial.println(m_curStartPtr);

  return totalWidth;
}