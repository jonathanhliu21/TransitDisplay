#ifndef USER_CONSTANTS_H
#define USER_CONSTANTS_H

#include <vector>

// comment this out below if you don't want to use a filter
// e.g. you want ALL operators to be included in the search radius provided
// It is **highly** recommended to use a filter to avoid long initialization times
// as transit.land API can be quite slow.
#define TRANSIT_USE_FILTER
#ifdef TRANSIT_USE_FILTER
// put vector of strings of onestop IDs of transit land **operators**
// Operators can be found here: https://www.transit.land/operators
// Example below includes BART and MUNI in SF Bay Area and LA Metro and Metrolink in Greater LA
#define TRANSIT_FILTER {"o-9q5-metro~losangeles", "o-9qh-metrolinktrains", "o-9q9-bart", "o-9q8y-sfmta"}
#endif

// This program works by assigning a name (which is displayed at the top) and pinning a location + radius
// The API scans for **all** routes and stops within the radius
// and then retrieves departures for **all** stops in the radius.
// This is why it is recommended to set a smaller radius (~50-100 meters).
// Radius units are in meters
// The example below scans a 100 meter radius around the Westwood/Weyburn stop in Westwood, Los Angeles, CA
#define TRANSIT_ZONE_NAME "Westwood / Weyburn"
#define TRANSIT_PIN_LAT 34.062591
#define TRANSIT_PIN_LON -118.445390
#define TRANSIT_PIN_RADIUS 100 // meters

#endif
