#include "frontend/Filter.h"

#include <Arduino.h>
#include <set>

namespace
{
  const char *METRO_LOS_ANGELES = "o-9q5-metro~losangeles";
  const char *BAY_AREA_RAPID_TRANSIT = "o-9q9-bart";

  const int LA_METRO_RAPID_COLOR = 0xC54858;
  const int LA_METRO_LOCAL_COLOR = 0xfa7343;
  const int COLOR_WHITE = 0xFFFFFF;
}

std::vector<DisplayRoute> Filter::modifyRoutes(const std::vector<DisplayRoute> &rts)
{
  std::vector<DisplayRoute> routes = rts;

  // first combine directions
  auto cmp = [](const DisplayRoute &a, const DisplayRoute &b)
  {
    if (a.agencyOnestopId == b.agencyOnestopId)
    {
      return a.name < b.name;
    }
    return a.agencyOnestopId < b.agencyOnestopId;
  };
  std::set<DisplayRoute, decltype(cmp)> unique_base_names(cmp);

  for (const DisplayRoute &r : routes)
  {
    // Find the position of the last hyphen '-'.
    // Arduino's lastIndexOf() returns -1 if the character is not found.
    DisplayRoute route = r;
    String line = r.name.c_str();
    int pos = line.lastIndexOf('-');

    if (pos != -1)
    {
      // Extract the suffix using the substring() method.
      String suffix = line.substring(pos + 1);

      // Check if the suffix is a valid cardinal direction.
      if (suffix == "N" || suffix == "S" || suffix == "E" || suffix == "W")
      {
        // It's a valid directional line, so extract the base name.
        route.name = line.substring(0, pos).c_str();
        unique_base_names.insert(route);
      }
      else
      {
        // Not a valid direction (e.g., "Red-Express"), so use the whole string.
        unique_base_names.insert(route);
      }
    }
    else
    {
      // No hyphen, so use the whole string.
      unique_base_names.insert(route);
    }
  }
  routes.assign(unique_base_names.begin(), unique_base_names.end());

  // truncate names
  for (int i = 0; i < routes.size(); i++)
  {
    routes[i].name = truncateRoute(routes[i].name);
  }

  // color LA metro buses
  modifyRoutesLAMetro(routes);

  // color buses
  for (int i = 0; i < routes.size(); i++)
  {
    DisplayRoute &route = routes[i];
    if (route.lineColor == 0 && route.textColor == 0xffffff)
    {
      route.lineColor = LA_METRO_RAPID_COLOR;
    }
    if (route.lineColor == 0 && route.textColor == 0)
    {
      route.lineColor = 0;
      route.textColor = COLOR_WHITE;
    }
  }

  return routes;
}

void Filter::modifyDeparture(DisplayDeparture &dep)
{
  modifyAgentSpecific(dep, dep.agencyOnestopId);

  dep.direction = truncateStop(dep.direction, true);
  dep.line = truncateRoute(dep.line);
}

std::string Filter::truncateStop(const std::string &name, const bool truncateDowntown)
{
  // We will work on a copy of the name so we don't modify the original reference.
  String result = name.c_str();

  // --- Rule 1: Cut off any line number / service and a dash at the beginning ---
  // This rule uses a combined heuristic to decide whether to truncate.
  int dashIndex = result.indexOf(" - ");

  // Only proceed if a dash is found.
  if (dashIndex != -1)
  {
    bool shouldTruncate = false;

    // Heuristic A: Check if the prefix is purely numeric (e.g., "2 - ...")
    String prefix = result.substring(0, dashIndex);
    prefix.trim(); // Remove whitespace for accurate checking

    bool prefixIsNumericOnly = true;
    if (prefix.length() > 0)
    {
      for (int i = 0; i < prefix.length(); i++)
      {
        if (!isDigit(prefix.charAt(i)))
        {
          prefixIsNumericOnly = false;
          break;
        }
      }
    }
    else
    {
      prefixIsNumericOnly = false;
    }

    if (prefixIsNumericOnly)
    {
      shouldTruncate = true;
    }

    // Heuristic B: Check if it looks like a long headsign (e.g., "... Downtown ... Station")
    // This is a fallback for non-numeric service names like "Metro E Line - ..."
    if (!shouldTruncate && (result.indexOf("Downtown") != -1 || result.indexOf("Station") != -1) && !result.endsWith("Downtown"))
    {
      shouldTruncate = true;
    }

    // If either heuristic passed, perform the truncation.
    if (shouldTruncate)
    {
      result = result.substring(dashIndex + 3); // Length of " - " is 3
    }
  }

  // Trim whitespace after every operation to keep the string clean.
  result.trim();

  // --- Rule 2: Cut off "Downtown" at the beginning ---
  if (truncateDowntown && result.startsWith("Downtown"))
  {
    // Take the substring that starts after the word "Downtown".
    result = result.substring(String("Downtown").length());
  }

  result.trim();

  // --- Rule 3: Cut off "Station" at the end ---
  if (result.endsWith("Station") && !result.endsWith("Union Station"))
  {
    // Take the substring from the beginning up to where "Station" starts.
    result = result.substring(0, result.length() - String("Station").length());
  }

  // Also cut off "Rapid" at the end
  if (result.endsWith("Rapid"))
  {
    // Take the substring from the beginning up to where "Rapid" starts.
    result = result.substring(0, result.length() - String("Rapid").length());
  }

  // Perform a final trim to clean up any trailing space and return the result.
  result.trim();

  return std::string(result.c_str());
}

std::string Filter::truncateRoute(const std::string &routeStr)
{
  String result = routeStr.c_str();

  // --- Cut off "Metro" at the beginning ---
  if (result.startsWith("Metro"))
  {
    // Take the substring that starts after the word "Metro".
    result = result.substring(String("Metro").length());
  }

  result.trim();

  // --- Cut off "Line" at the end ---
  if (result.endsWith("Line"))
  {
    // Take the substring from the beginning up to where "Line" starts.
    result = result.substring(0, result.length() - String("Line").length());
  }

  // --- Truncate stuff like "J Line formerly Silver Line with services 910 and 950 to Harbor Gateway and San Pedro respectively" ---
  // Also avoids incorrectly shortening names like "Rapid 6".
  int firstSpaceIndex = result.indexOf(' ');

  // Only check if a space exists.
  if (firstSpaceIndex != -1)
  {
    bool shouldTruncate = false;

    // Get the parts before and after the first space.
    String prefix = result.substring(0, firstSpaceIndex);
    String suffix = result.substring(firstSpaceIndex);
    suffix.trim(); // Clean up suffix for inspection.

    // Heuristic A (New): If the first word is a single letter or digit, truncate.
    if (prefix.length() == 1 && (isAlpha(prefix.charAt(0)) || isDigit(prefix.charAt(0))))
    {
      shouldTruncate = true;
    }

    // Heuristic B (Old): If not, truncate only if the suffix is "complex".
    if (!shouldTruncate)
    {
      bool isComplex = false;
      // Check if the suffix contains anything other than digits.
      for (int i = 0; i < suffix.length(); i++)
      {
        if (!isDigit(suffix.charAt(i)))
        {
          isComplex = true; // Found a non-digit character, so it's complex.
          break;
        }
      }
      if (isComplex)
      {
        shouldTruncate = true;
      }
    }

    // If either heuristic decided we should truncate, do it.
    if (shouldTruncate)
    {
      result = prefix; // result becomes just the prefix
    }
  }

  // Perform a final trim to clean up any trailing space and return the result.
  result.trim();
  return std::string(result.c_str());
}

void Filter::modifyAgentSpecific(DisplayDeparture &dep, const std::string &agencyOnestopId)
{
  if (agencyOnestopId == METRO_LOS_ANGELES)
  {
    modifyDepLAMetro(dep);
  }
  else if (agencyOnestopId == BAY_AREA_RAPID_TRANSIT)
  {
    modifyDepBART(dep);
  }
}

void Filter::modifyRoutesLAMetro(std::vector<DisplayRoute> &routes)
{
  // color LA metro buses
  for (int i = 0; i < routes.size(); i++)
  {
    DisplayRoute &route = routes[i];
    if (route.agencyOnestopId == METRO_LOS_ANGELES)
    {
      if (route.lineColor == 0 && route.textColor == 0xffffff)
      {
        route.lineColor = LA_METRO_RAPID_COLOR;
      }
      if (route.lineColor == 0 && route.textColor == 0)
      {
        route.lineColor = LA_METRO_LOCAL_COLOR;
        route.textColor = COLOR_WHITE;
      }
    }
  }
}

void Filter::modifyDepLAMetro(DisplayDeparture &res)
{
  if (res.routeColor == 0 && res.textColor == 0xffffff)
  {
    res.routeColor = LA_METRO_RAPID_COLOR;
  }
  if (res.routeColor == 0 && res.textColor == 0)
  {
    res.routeColor = LA_METRO_LOCAL_COLOR;
    res.textColor = COLOR_WHITE;
  }
}

void Filter::modifyDepBART(DisplayDeparture &res)
{
  if (res.direction == "SFO / SF / Antioch")
  {
    res.direction = "Antioch";
  }
}