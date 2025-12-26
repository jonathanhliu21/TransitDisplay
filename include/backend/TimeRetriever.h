#ifndef TIME_RETRIEVER_H
#define TIME_RETRIEVER_H

#include <time.h>

class TimeRetriever
{
public:
  TimeRetriever();

  void sync();
  time_t getCurTime() const;
  time_t timegmUTC(struct tm *timeinfo);

private:
  time_t espRetrieveTime();

  time_t m_startTimeSeconds;
  time_t m_startTimeUTC;
};

#endif