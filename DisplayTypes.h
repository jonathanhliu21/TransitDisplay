#ifndef DISPLAY_TYPES_H
#define DISPLAY_TYPES_H

#include <Arduino.h>

struct BridgeDeparture
{
    String direction;
    int mins;
    String line;
    int textColor;
    int routeColor;
    int delayColor;
};

#endif