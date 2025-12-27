#ifndef ZONE_LIST_DISPLAYER_H
#define ZONE_LIST_DISPLAYER_H

#include <TFT_eSPI.h>
#include <backend/TransitZone.h>

class ZoneListDisplayer
{
public:
  ZoneListDisplayer(TFT_eSPI *tft,
                    const uint8_t *fontRegular,
                    const uint8_t *fontLarge);

  void drawConnecting();
  void drawNoZonesFound();
  void drawZone(TransitZone *zone, TransitZone *next);

private:
  TFT_eSPI *m_tft;
  const uint8_t *m_fontRegular;
  const uint8_t *m_fontLarge;
};

#endif