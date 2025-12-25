#ifndef CONSTANTS_H
#define CONSTANTS_H

/**
 * Used for constants used across files, or pins
 *
 * File-specific constants can be found in respective cpp file
 */

namespace Constants
{
  inline constexpr const char *API_CALLER_STATUS_KEY = "APICallerStatus";
  inline constexpr const char *API_HTTP_STATUS_KEY = "APICallerHTTPStatus";
  inline constexpr const char *API_DESERIALIZE_ERROR_KEY = "APICallerDeserializeError";

  inline constexpr int ROUTE_ERROR_PIN = 33;
  inline constexpr int STOP_ERROR_PIN = 25;
  inline constexpr int DEPARTURE_ERROR_PIN = 26;
  inline constexpr int RATE_LIMIT_PIN = 27;

  inline constexpr int MAX_PAGES_PROCESSED = 5;
}

#endif