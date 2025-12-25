#include "BaseRetriever.h"

#include <Arduino.h>
#include <string>
#include <ArduinoJson.h>

#include "APICaller.h"
#include "Constants.h"

namespace
{
  const int RETRY_DELAY = 10000; // ms
  const int PING_DELAY = 1000;   // ms
  const char *TRANSIT_LAND_URL_PREFIX = "https://transit.land";
}

BaseRetriever::BaseRetriever(
    APICaller *caller, const std::string &endpoint, const int maxPages, const int errorPin)
    : m_caller{caller}, m_endpoint{endpoint}, m_maxPages{maxPages}, m_errorPin{errorPin} {}

std::string BaseRetriever::getEndpoint() const { return m_endpoint; }
void BaseRetriever::setEndpoint(const std::string &endpoint) { m_endpoint = endpoint; }

bool BaseRetriever::loopRequest(
    const JsonDocument &filter, const std::string &arrKeyName, const int nestingLimit)
{
  int loopCnt = 0;
  std::string curEndpoint = m_endpoint;
  while (curEndpoint.length() > 0 && loopCnt < m_maxPages)
  {
    writePinIfExists(LOW);

    // fetch API
    JsonDocument responseDoc = m_caller->call(curEndpoint, filter, nestingLimit);

    // print error, if any, and fail
    if (responseDoc[Constants::API_CALLER_STATUS_KEY] != static_cast<int>(APICallerStatus::STATUS_OK))
    {
      writePinIfExists(HIGH);

      int httpCode = responseDoc[Constants::API_HTTP_STATUS_KEY];

      // retry in 10 seconds if read timeout
      if (httpCode == HTTPC_ERROR_READ_TIMEOUT)
      {
        Serial.println(" (Timeout). Retrying...");
        delay(RETRY_DELAY);
        continue;
      }

      // activate rate limiting pin if we are rate limited
      if (httpCode == HTTP_CODE_TOO_MANY_REQUESTS)
      {
        digitalWrite(Constants::RATE_LIMIT_PIN, HIGH);
      }

      Serial.println(("Endpoint failed: " + m_endpoint).c_str());
      serializeJson(responseDoc, Serial);
      return false;
    }

    // retrieve next page
    if (responseDoc["meta"].isNull() || !responseDoc["meta"]["next"].is<const char *>())
    {
      curEndpoint = "";
    }
    else
    {
      curEndpoint = responseDoc["meta"]["next"].as<std::string>();
      curEndpoint = curEndpoint.substr(strlen(TRANSIT_LAND_URL_PREFIX));
    }

    // find key
    if (responseDoc[arrKeyName].isNull())
    {
      writePinIfExists(HIGH);
      Serial.println(("Key " + arrKeyName + " is not there").c_str());
      return false;
    }

    // loop through each element array
    // each element represents one route, one stop, or one departure
    JsonArrayConst arr = responseDoc[arrKeyName].as<JsonArrayConst>();
    int size = arr.size();
    for (int i = 0; i < size; i++)
    {
      if (arr[i].isNull())
        continue;

      JsonVariantConst elementDoc = arr[i].as<JsonVariantConst>();
      parseOneElement(elementDoc);
    }

    delay(PING_DELAY); // so we don't overwhelm server
    loopCnt++;
  }

  return true;
}

void BaseRetriever::writePinIfExists(int state)
{
  if (m_errorPin == -1)
    return;
  digitalWrite(m_errorPin, state);
}