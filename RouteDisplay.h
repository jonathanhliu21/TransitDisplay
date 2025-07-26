#ifndef ROUTE_DISPLAY_H
#define ROUTE_DISPLAY_H

#include <TFT_eSPI.h>
#include <Arduino.h>
#include <vector>
#include "RouteTable.h"
#include "DisplayHelpers.h"

class RouteDisplay
{
public:
  RouteDisplay(TFT_eSPI *tft);

  void setRoutes(std::vector<Route> routes);

  void cycle();

private:
  TFT_eSPI *m_tft;
  int m_curStartPtr;

  std::vector<Route> m_routes;

  int preCalculation();
};

#endif