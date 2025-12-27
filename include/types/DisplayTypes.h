#ifndef DISPLAY_TYPES_H
#define DISPLAY_TYPES_H

#include <string>

#include "types/TransitTypes.h"

using DisplayRoute = Route;

struct DisplayDeparture
{
  std::string agencyOnestopId;
  std::string direction;
  std::string line;
  int mins;
  int textColor;
  int routeColor;
  int delayColor;
};

#endif