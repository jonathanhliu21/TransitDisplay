#ifndef FILTER_H
#define FILTER_H

#include "types/DisplayTypes.h"
#include <string>
#include <vector>

class Filter
{
public:
  static std::vector<DisplayRoute> modifyRoutes(const std::vector<DisplayRoute> &routes);
  static void modifyDeparture(DisplayDeparture &dep);

private:
  static std::string truncateStop(const std::string &name, const bool truncateDowntown);
  static std::string truncateRoute(const std::string &routeStr);
  static void modifyAgentSpecific(DisplayDeparture &dep, const std::string &agencyOnestopId);

  // ROUTES: TRANSIT-SPECIFIC AGENCIES BELOW
  static void modifyRoutesLAMetro(std::vector<DisplayRoute> &routes);

  // DEPARTURES: TRANSIT-SPECIFIC AGENCIES BELOW
  static void modifyDepLAMetro(DisplayDeparture &dep);
  static void modifyDepBART(DisplayDeparture &dep);
};

#endif