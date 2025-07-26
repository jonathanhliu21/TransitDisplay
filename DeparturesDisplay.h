#ifndef DEPARTURES_DISPLAY_H
#define DEPARTURES_DISPLAY_H

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "DisplayTypes.h"

class DeparturesDisplay
{
public:
  DeparturesDisplay(TFT_eSPI *tft);

  void clearDepartures();
  void setDepartures(std::vector<BridgeDeparture> departures);

  void cycle();

private:
  TFT_eSPI *m_tft;
  std::vector<BridgeDeparture> m_departures;

  String truncateText(const String &text, int maxWidth);
};

#endif