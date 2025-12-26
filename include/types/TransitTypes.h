#ifndef TRANSIT_TYPES_H
#define TRANSIT_TYPES_H

#include <string>
#include <time.h>

struct Route
{
  std::string onestopId;
  std::string name;
  int lineColor;
  int textColor;
  std::string agencyOnestopId;
};

struct Stop
{
  std::string onestopId;
  std::string name;
};

struct Departure
{
  Route route;
  Stop stop;
  std::string direction;
  time_t expectedTimestamp;
  time_t actualTimestamp;
  bool isRealTime;
  std::string agencyOnestopId;
  int delay;
  bool isValid;
};

#endif