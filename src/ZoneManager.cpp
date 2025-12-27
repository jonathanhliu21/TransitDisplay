#include "ZoneManager.h"

#include "frontend/Filter.h"

namespace
{
  const int DEPARTURE_DISP_REFRESH_RATE = 10000; // ms
  const int ROUTE_DISP_REFRESH_RATE = 3000;      // ms

  const int DEPARTURE_API_CALL_REFRESH_PERIOD = 30000; // ms
  const int TIME_SYNC_REFRESH_PERIOD = 300000;         // ms

  const int ON_TIME_COLOR = 0x00FF00;
  const int DELAYED_COLOR = 0xFF0000;
  const int EARLY_COLOR = 0xFFFF00;
  const int NO_RT_INFO_COLOR = 0xFFFFFF;
  const int DELAY_CUTOFF = 60;
}

ZoneManager::ZoneManager(
    TransitZone *zone,
    TFT_eSPI *tft,
    TimeRetriever *timeRetriever,
    const Whitelist &whitelist,
    const uint8_t *fontRegular,
    const uint8_t *fontLarge)
    : m_zone{zone},
      m_timeRetriever{timeRetriever},
      m_whitelist{whitelist},
      m_displayer{
          zone->getName(),
          tft,
          fontRegular,
          fontLarge,
          ROUTE_DISP_REFRESH_RATE,
          DEPARTURE_DISP_REFRESH_RATE,
      },
      m_lastSyncedTime{0}
{
}

ZoneManager::~ZoneManager()
{
  stop();
}

void ZoneManager::init()
{
  // Prevent creating multiple tasks if int() is called more than once.
  if (m_retrieval_thread_handle != NULL)
  {
    return;
  }

  m_timeRetriever->sync(); // syncs time to NTP
  m_lastSyncedTime = millis();
  m_displayer.drawInitializing();

  // if zone not initialized then get zone routes
  if (!m_zone->isInitialized())
  {
    m_zone->init(m_whitelist);
    m_displayer.setRoutes(
        Filter::modifyRoutes(m_zone->getRoutes().getDisplayRouteList()));
  }

  m_zone->callDeparturesAPI();
  std::vector<DisplayDeparture> displayDepartureList = m_zone->getDepartures().getDisplayDepartureList(
      m_timeRetriever->getCurTime(),
      ON_TIME_COLOR,
      DELAYED_COLOR,
      EARLY_COLOR,
      NO_RT_INFO_COLOR,
      DELAY_CUTOFF);

  for (int i = 0; i < displayDepartureList.size(); i++)
  {
    Filter::modifyDeparture(displayDepartureList[i]);
  }

  m_displayer.setDepartures(displayDepartureList);

  // start thread
  xTaskCreate(
      retrievalTaskRunner,       // Function to implement the task
      "DepartureRetrievalTask",  // Name of the task
      8192,                      // Stack size in words
      this,                      // Task input parameter (pointer to this instance)
      1,                         // Priority of the task
      &m_retrieval_thread_handle // Task handle to keep track of created task
  );

  m_displayer.cycle();
}

void ZoneManager::mainThreadLoop()
{
  // displayer is shared
  std::lock_guard<std::mutex> lock(m_displayerMtx);
  m_displayer.loop();
}

void ZoneManager::stop()
{
  if (m_retrieval_thread_handle != NULL)
  {
    vTaskDelete(m_retrieval_thread_handle);
    m_retrieval_thread_handle = NULL; // Set handle to NULL to indicate task is stopped.
    Serial.println("Departure retrieval task stopped.");
  }
}

void ZoneManager::drawAreYouSure()
{
  m_displayer.drawAreYouSure();
}

void ZoneManager::cycleDisplay()
{
  m_displayer.cycle();
}

void ZoneManager::retrievalTaskRunner(void *pvParameters)
{
  ZoneManager *inst = static_cast<ZoneManager *>(pvParameters);
  inst->bgTaskLoop();
}

void ZoneManager::bgTaskLoop()
{
  unsigned long last_retrieval_time = millis();
  while (true)
  {
    if (millis() - last_retrieval_time >= DEPARTURE_API_CALL_REFRESH_PERIOD)
    {
      last_retrieval_time = millis();

      m_zone->callDeparturesAPI();
      std::vector<DisplayDeparture> displayDepartureList = m_zone->getDepartures().getDisplayDepartureList(
          m_timeRetriever->getCurTime(),
          ON_TIME_COLOR,
          DELAYED_COLOR,
          EARLY_COLOR,
          NO_RT_INFO_COLOR,
          DELAY_CUTOFF);

      for (int i = 0; i < displayDepartureList.size(); i++)
      {
        Filter::modifyDeparture(displayDepartureList[i]);
      }
      m_displayer.setDepartures(displayDepartureList);
    }

    if (millis() - m_lastSyncedTime >= TIME_SYNC_REFRESH_PERIOD)
    {
      m_lastSyncedTime = millis();
      m_timeRetriever->sync();
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void ZoneManager::safeSetDisplayDeps(const std::vector<DisplayDeparture> &deps)
{
  // displayer is shared
  std::lock_guard<std::mutex> lock(m_displayerMtx);
  m_displayer.setDepartures(deps);
}