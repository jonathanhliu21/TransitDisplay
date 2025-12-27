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
    {"Westwood / Weyburn", 34.062591, -118.445390, 100},
    {"7th St / Metro Center", 34.0489, -118.2588, 100},
};

// set userWhiteListActive to false if you don't want to use a filter
// e.g. you want ALL operators to be included in the search radius provided
// It is **highly** recommended to use a filter to avoid long initialization times
// as transit.land API can be quite slow.
bool userWhiteListActive = true;

// put vector of strings of onestop IDs of transit land **operators**
// Operators can be found here: https://www.transit.land/operators
// Example below includes LA Metro and Metrolink in Greater LA and BART and Caltrain in SF Bay Area
std::vector<std::string> userWhiteList = {
    "o-9q5-metro~losangeles",
    "o-9qh-metrolinktrains",
    "o-9q9-bart",
    "o-9q9-caltrain",
    "o-dr5r-nyct",
    "o-c23-soundtransit"};

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#endif