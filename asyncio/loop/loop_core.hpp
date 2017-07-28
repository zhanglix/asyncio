#pragma once

#include <asyncio/common.hpp>

BEGIN_ASYNCIO_NAMESPACE;
class TimerHandle;
class LoopCore {
public:
  virtual void runOneIteration() = 0;
  virtual size_t activeHandlesCount() = 0;
  virtual uint64_t time() = 0;
  virtual TimerHandle *callSoon(void (*callback)(TimerHandle *),
                                void *data) = 0;
  virtual TimerHandle *callSoonThreadSafe(void (*callback)(TimerHandle *),
                                          void *data) = 0;
  virtual TimerHandle *callLater(uint64_t milliseconds,
                                 void (*callback)(TimerHandle *),
                                 void *data) = 0;
  virtual bool cancelTimer(TimerHandle *handle) = 0;
  virtual void recycleTimerHandle(TimerHandle *handle) = 0;
};

END_ASYNCIO_NAMESPACE;