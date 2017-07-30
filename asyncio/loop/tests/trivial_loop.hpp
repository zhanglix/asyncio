#pragma once
#include <vector>

#include "../loop_core.hpp"
#include "../timer_handle.hpp"
#include <asyncio/common.hpp>

BEGIN_ASYNCIO_NAMESPACE;

class TrivialLoop : public LoopCore {
public:
  TrivialLoop() : _now(0) {}
  virtual ~TrivialLoop() {}
  virtual void runOneIteration() override;
  virtual void close() override;
  virtual size_t activeHandlesCount() override;
  virtual uint64_t time() override;
  virtual TimerHandle *callSoon(TimerCallback callback, void *data) override;
  virtual TimerHandle *callSoonThreadSafe(TimerCallback callback,
                                          void *data) override;
  virtual TimerHandle *callLater(uint64_t milliseconds, TimerCallback callback,
                                 void *data) override;
  virtual bool cancelTimer(TimerHandle *handle) override;
  virtual void recycleTimerHandle(TimerHandle *handle) override;

private:
  class TrivialTimerHandle : public TimerHandle {
  public:
    TimerCallback callback;
  };
  std::vector<TrivialTimerHandle *> _timers;
  uint64_t _now;
};
END_ASYNCIO_NAMESPACE;