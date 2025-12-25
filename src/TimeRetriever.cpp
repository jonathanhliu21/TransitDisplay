#include "TimeRetriever.h"

#include <Arduino.h>
#include <time.h>

namespace
{
  const char *TIME_URL = "pool.ntp.org";
}

TimeRetriever::TimeRetriever() : m_startTimeSeconds{0}, m_startTimeUTC{0} {}

void TimeRetriever::sync()
{
  m_startTimeUTC = espRetrieveTime();
  m_startTimeSeconds = millis() / 1000.0;
}

time_t TimeRetriever::getCurTime() const
{
  // last UTC time recorded + time elapsed since last UTC time recorded
  return m_startTimeUTC + millis() / 1000.0 - m_startTimeSeconds;
}

time_t TimeRetriever::espRetrieveTime()
{
  configTime(0, 0, TIME_URL);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Cannot get local time");
    return -1;
  }

  return timegmUTC(&timeinfo);
}

time_t TimeRetriever::timegmUTC(struct tm *timeinfo)
{
  time_t result;

  // 1. Save the current TZ environment variable
  char *tz_original = getenv("TZ");

  // 2. Set the timezone to UTC
  setenv("TZ", "", 1); // An empty string or "UTC0" means UTC
  tzset();             // Apply the new timezone

  // 3. Call mktime(), which now thinks the struct is in UTC
  result = mktime(timeinfo);

  // 4. Restore the original timezone
  if (tz_original)
  {
    setenv("TZ", tz_original, 1);
  }
  else
  {
    unsetenv("TZ");
  }
  tzset(); // Apply the original timezone back

  return result;
}