#ifndef TIME_RETRIEVER_H
#define TIME_RETRIEVER_H

#include <ctime>

class TimeRetriever
{
public:
  TimeRetriever();

  bool sync();
  std::time_t getCurTime() const;
  static std::time_t timegmUTC(struct tm *timeinfo);

private:
  static std::time_t espRetrieveTime();

  std::time_t m_startTimeSeconds;
  std::time_t m_startTimeUTC;
};

#endif