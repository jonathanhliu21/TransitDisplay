#ifndef BASE_RETRIEVER_H
#define BASE_RETRIEVER_H

#include "APICaller.h"
#include <string>
#include <ArduinoJson.h>

class BaseRetriever
{
public:
  BaseRetriever(APICaller *caller,
                const std::string &endpoint,
                const int maxPages,
                const int errorPin = -1);

  virtual bool retrieve() = 0;
  std::string getEndpoint() const;
  void setEndpoint(const std::string &endpoint);

protected:
  bool loopRequest(const JsonDocument &filter,
                   const std::string &arrKeyName,
                   const int nestingLimit = ARDUINOJSON_DEFAULT_NESTING_LIMIT);
  virtual void parseOneElement(JsonVariantConst &elementDoc) = 0;

private:
  APICaller *m_caller;
  std::string m_endpoint;
  int m_maxPages;
  int m_errorPin;

  void writePinIfExists(int state);
};

#endif