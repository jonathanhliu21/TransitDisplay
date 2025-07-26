#include "DeparturesDisplay.h"

// =======================================================
// 1. LAYOUT & FONT CONSTANTS (Adjust these to your liking)
// =======================================================

#include "DisplayConstants.h"
#include "Overpass_Regular12.h"
#include "DisplayHelpers.h"


// =======================================================
// 2. CLASS IMPLEMENTATION
// =======================================================

DeparturesDisplay::DeparturesDisplay(TFT_eSPI *tft) : m_tft(tft) {
  // Constructor can be empty
}

/**
 * @brief Clears the entire area where departures are drawn.
 */
void DeparturesDisplay::clearDepartures() {
  // Calculate the total height of the 5 rows plus spacing to clear the exact area
  int clearHeight = (5 * (DEPARTURES_ROW_HEIGHT + DEPARTURES_ROW_SPACING));
  m_tft->fillRect(0, DEPARTURES_START_Y, DISPLAY_WIDTH, clearHeight, TFT_BLACK);
}

/**
 * @brief Updates the list of departures to be displayed.
 * @param departures A vector of BridgeDeparture structs.
 */
void DeparturesDisplay::setDepartures(std::vector<BridgeDeparture> departures) {
  m_departures = departures;
}

/**
 * @brief Draws the departure list to the screen. Call this after setDepartures.
 */
void DeparturesDisplay::cycle() {
  // First, clear the area of the old list
  clearDepartures();

  // Load the custom font for drawing.
  // This must match the name of the .vlw file you created.
  m_tft->loadFont(Overpass_Regular12);

  // Serial.println(m_tft->textWidth("Yellow-N"));
  if (m_departures.empty()) {
    m_tft->setTextDatum(MC_DATUM);
    m_tft->setTextColor(TFT_WHITE);

    // Calculate the center of the departures area to display the message
    int centerX = DISPLAY_WIDTH / 2;
    int centerY = DEPARTURES_START_Y + (DISPLAY_HEIGHT - DEPARTURES_START_Y) / 2;
    
    m_tft->drawString("No departures found", centerX, centerY);
    m_tft->unloadFont();
    return;
  }

   // 1. Calculate the width of the widest possible minutes string to reserve that space.
  int maxMinsTextWidth = m_tft->textWidth("99 min");
  // 2. Calculate the available space for the direction text dynamically.
  int maxDirectionWidth = COL_MINS_X - COL_DIRECTION_X - maxMinsTextWidth - 15; // 15px gap for safety

  // Determine how many departures to show (up to a maximum of 5)
  int numToDisplay = min((int)m_departures.size(), 5);

  for (int i = 0; i < numToDisplay; i++) {
    BridgeDeparture dep = m_departures[i];
    if (dep.mins >= 100) break; // don't render > 100 mins

    // Calculate the Y position for the top of the current row
    int currentY = DEPARTURES_START_Y + (i * (DEPARTURES_ROW_HEIGHT + DEPARTURES_ROW_SPACING));
    // Calculate the vertical center for text alignment
    int textY = currentY + DEPARTURES_ROW_HEIGHT / 2 + 2; // +2px for vertical text centering with VLW fonts

    // --- Column 1: Line Name (Constrained Width) ---
    // Calculate the ideal width with padding
    int idealWidth = m_tft->textWidth(dep.line) + ROUTE_PADDING;
    // Constrain the button width to the maximum allowed
    int buttonWidth = min(idealWidth, DEPARTURES_MAX_LINE_BUTTON_WIDTH);
    // Determine the available space for text inside the constrained button
    int textSpace = buttonWidth - DEPARTURES_MIN_PADDING;
    // Get the final text, truncated if necessary
    String lineText = truncateText(dep.line, textSpace);
    int buttonX = COL_LINE_CENTER_X - (buttonWidth / 2);

    // Draw the colored background button
    m_tft->fillRoundRect(buttonX, currentY, buttonWidth, DEPARTURES_ROW_HEIGHT, 5, hexToRGB565(dep.routeColor));
    // Set text properties and draw the line name centered inside the button
    m_tft->setTextDatum(MC_DATUM);
    m_tft->setTextColor(hexToRGB565(dep.textColor));
    m_tft->drawString(lineText, COL_LINE_CENTER_X, textY + DEPARTURES_TEXT_Y_OFFSET);

    // --- Column 2: Direction ---
    String directionText = truncateText(dep.direction, maxDirectionWidth);

    // Set text properties and draw the direction, left-aligned to its column
    m_tft->setTextDatum(ML_DATUM);
    m_tft->setTextColor(TFT_WHITE); // A standard color for directions
    m_tft->drawString(directionText, COL_DIRECTION_X, textY);

    // --- Column 3: Minutes ---
    // Set text properties and draw the minutes, right-aligned to its column
    m_tft->setTextDatum(MR_DATUM);
    m_tft->setTextColor(hexToRGB565(dep.delayColor));
    String minsText = String(dep.mins) + " min";
    if (dep.mins <= 0) {
      minsText = "Now";
    }
    m_tft->drawString(minsText, COL_MINS_X, textY);
  }

  // Unload the font to free up RAM
  m_tft->unloadFont();
}

/**
 * @brief Truncates text with an ellipsis if it exceeds a max pixel width.
 */
String DeparturesDisplay::truncateText(const String& text, int maxWidth) {
  // Return the original text if it already fits
  if (m_tft->textWidth(text) <= maxWidth) {
    return text;
  }

  // Width of the ellipsis is needed for calculations
  int ellipsisWidth = m_tft->textWidth("...");

  // If even the ellipsis doesn't fit, there's nothing we can do
  if (ellipsisWidth > maxWidth) {
    return "";
  }
  
  // Iterate backwards from the end of the string
  for (int i = text.length() - 1; i > 0; i--) {
    String sub = text.substring(0, i);
    if (m_tft->textWidth(sub) <= (maxWidth - ellipsisWidth)) {
      return sub + "...";
    }
  }

  // Fallback for very short max widths
  return "...";
}