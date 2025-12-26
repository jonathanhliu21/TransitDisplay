#ifndef API_CALLER_H
#define API_CALLER_H

#include <string>
#include <HTTPClient.h>
#include <ArduinoJson.h>

enum class APICallerStatus
{
  STATUS_OK,
  HTTP_ERROR,
  DESERIALIZE_ERROR
};

class APICaller
{
public:
  APICaller(const std::string &apiKey);

  JsonDocument call(const std::string &endpoint,
                    const JsonDocument &filter,
                    const int nestingLimit = ARDUINOJSON_DEFAULT_NESTING_LIMIT,
                    const bool attachApiKey = true);

private:
  std::string m_apiKey;
  HTTPClient m_client;
};

#endif