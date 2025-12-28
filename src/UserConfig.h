#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#include <vector>
#include <string>

#include "Configuration.h"

// vvv EDIT BELOW TO DESIRED CONFIGURATION vvv

// Create a list of names, latitudes, and longitudes, with corresponding coordinates
// Each element should be in tuple form: {name: str, lat: float, lon: float, radius: float}
// This program works by assigning a name (which is displayed at the top) and pinning a location + radius
// The API scans for **all** routes and stops within the radius
// and then retrieves departures for **all** stops in the radius.
// This is why it is recommended to set a smaller radius (~50-100 meters).
// Radius units are in **meters**
//   Name               , Latitude , Longitude  , Radius (m)
UserTransitZoneList userTransitZoneList = {

    {"Montgomery", 37.789323243900085, -122.40135250129704, 100},      // Financial Dist. San Francisco
    {"Westwood / Weyburn", 34.062591, -118.445390, 100},               // Westwood, Los Angeles
    {"7th St / Metro Center", 34.0489, -118.2588, 100},                // Downtown LA
    {"Times Sq-42 St", 40.75519779257256, -73.98687885217348, 100},    // Times Square, New York
    {"Westlake", 47.61161369567222, -122.33667514831642, 100},         // Downtown Seattle
    {"Downtown Crossing", 42.35548291630822, -71.06034670033964, 100}, // Downtown Boston
    {"Clark/Lake", 41.88567545397383, -87.63141011522981, 100}         // Chicago Loop
};

// set userWhiteListActive to false if you don't want to use a filter (whitelist)
// e.g. you want ALL operators to be included in the search radius provided
// It is **highly** recommended to use a filter to avoid long initialization times
// as transit.land API can be quite slow.
bool userWhiteListActive = true;

// put vector of strings of onestop IDs of transit land **operators**
// Operators can be found here: https://www.transit.land/operators
std::vector<std::string> userWhiteList = {
    "o-9q9-bart",                   // BART (SF Bay Area)
    "o-9q8y-sfmta",                 // SFMTA (San Francisco)
    "o-9q5-metro~losangeles",       // LA Metro (Los Angeles)
    "o-dr5r-nyct",                  // MTA (New York City)
    "o-c23-soundtransit",           // Sound Transit (Seattle)
    "o-drt-mbta",                   // MBTA (Boston)
    "o-dp3-chicagotransitauthority" // CTA (Chicago)
};

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#endif