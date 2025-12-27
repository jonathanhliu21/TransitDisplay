#ifndef DEPARTURES_DISPLAYER_H
#define DEPARTURES_DISPLAYER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>

#include "types/DisplayTypes.h"
#include "frontend/BaseDisplayer.h"

class DeparturesDisplayer : public BaseDisplayer
{
public:
  DeparturesDisplayer(TFT_eSPI *tft, const uint8_t *fontRegular);

  void drawBlankDepartureSpace();
  void setDepartures(const std::vector<DisplayDeparture> &departures);

  void cycle();

private:
  TFT_eSPI *m_tft;
  const uint8_t *m_fontRegular;
  std::vector<DisplayDeparture> m_departures;

  std::string truncateText(const std::string &text, int maxWidth);
};

#endif