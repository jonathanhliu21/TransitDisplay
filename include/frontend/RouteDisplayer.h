#ifndef ROUTE_DISPLAYER_H
#define ROUTE_DISPLAYER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>

#include "types/DisplayTypes.h"
#include "frontend/BaseDisplayer.h"

class RouteDisplayer : public BaseDisplayer
{
public:
  RouteDisplayer(TFT_eSPI *tft, const uint8_t *fontRegular);

  void setRoutes(std::vector<DisplayRoute> routes);
  virtual void cycle() override;

private:
  TFT_eSPI *m_tft;
  const uint8_t *m_fontRegular;
  int m_curStartPtr;
  std::vector<DisplayRoute> m_displayRoutes;

  int preCalculation();
};

#endif