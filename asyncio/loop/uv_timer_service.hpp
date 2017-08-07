#pragma once

#include <mutex>
#include <uv.h>

#include <asyncio/common.hpp>

#include "loop_core.hpp"
#include "uv_service.hpp"

BEGIN_ASYNCIO_NAMESPACE;
class UVTimerService;

class UVTimerHandle : public UVTimerHandleImp {
public:
  UVTimerHandle(UVTimerService *service);
  ~UVTimerHandle();

  void reset(uint64_t later, TimerCallback callback, void *data);

protected:
  virtual void doStartTimer() override; // eg: notify service
  virtual void doStopTimer() override;  // eg: notify service

  void uvTimerInit();
  void uvTimerStart(uint64_t later);
  void uvTimerStop();
  void close();

protected:
  UVTimerService *_service;
  uv_timer_t _uv_timer;
  uint64_t _later;
};

class UVTimerService : public UVService {
public:
  UVTimerService(uv_loop_t *uvLoop);

  TimerHandle *callLater(uint64_t later, TimerCallback callback, void *data) {
    auto h = new UVTimerHandle(this);
    h->reset(later, callback, data);
    h->setupTimer();
    return (TimerHandle *)h;
  }
};
END_ASYNCIO_NAMESPACE;