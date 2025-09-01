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
// Example below includes LA Metro and Metrolink in Greater LA and BART and Caltrain in SF Bay Area
#define TRANSIT_FILTER {"o-9q5-metro~losangeles", "o-9qh-metrolinktrains", "o-9q9-bart", "o-9q9-caltrain"}
#endif

// Create a list of names, latitudes, and longitudes, with corresponding coordinates
// Each element should be in tuple form: {name: str, lat: float, lon: float, radius: float}
// This program works by assigning a name (which is displayed at the top) and pinning a location + radius
// The API scans for **all** routes and stops within the radius
// and then retrieves departures for **all** stops in the radius.
// This is why it is recommended to set a smaller radius (~50-100 meters).
// Radius units are in **meters**
  // Name               , Latitude , Longitude  , Radius (m)
#define TRANSIT_ZONES { \
  { "Westwood / Weyburn", 34.062591, -118.445390, 100 }, \
  { "7th St / Metro Center", 34.0489, -118.2588, 100 }, \
  { "Embarcadero", 37.7928, -122.3970, 100 }, \
  { "Millbrae", 37.6002, -122.3867, 100 }, \
  { "Mountain View", 37.3945, -122.0769, 100 } \
}

#endif
