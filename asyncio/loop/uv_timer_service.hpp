#pragma once

#include <mutex>
#include <uv.h>

#include <asyncio/common.hpp>

#include "loop_core.hpp"
#include "timer_handle.hpp"

BEGIN_ASYNCIO_NAMESPACE;
class UVTimerService;
class TimerHandleImp : public TimerHandle {
public:
  TimerHandleImp(TimerCallback callback = nullptr, void *data = nullptr) {
    reset(callback, data);
  }
  void reset(TimerCallback callback, void *data) {
    _callback = callback;
    TimerHandle::reset(data);
  }

  void processEntry() { process(); }  // promote protected to public
  void setupTimer() { startTimer(); } // promote protected to public

protected:
  virtual bool executeTimer() override {
    (*_callback)(this);
    return true;
  };

  TimerCallback _callback;
};

class UVTimerHandle : public TimerHandleImp {
public:
  UVTimerHandle(UVTimerService *service);
  ~UVTimerHandle();

  void reset(uint64_t later, TimerCallback callback, void *data) {
    _later = later;
    TimerHandleImp::reset(callback, data);
  }

protected:
  virtual void doStartTimer() override; // eg: notify service
  virtual void doStopTimer() override;  // eg: notify service

  void uvTimerInit();
  void uvTimerStart(uint64_t later);
  void uvTimerStop();
  void close();

  UVTimerService *_service;
  uv_timer_t _uv_timer;
  uint64_t _later;
};

class UVTimerService {
public:
  UVTimerService(uv_loop_t *uvLoop);

  virtual void addHandle() { ++_activeHandles; }
  virtual void subHandle() { --_activeHandles; }
  size_t activeHandlesCount() { return _activeHandles; }

  TimerHandle *callLater(uint64_t later, TimerCallback callback, void *data) {
    auto h = new UVTimerHandle(this);
    h->reset(later, callback, data);
    h->setupTimer();
    return (TimerHandle *)h;
  }
  void startTimer(UVTimerHandle *) { addHandle(); }
  void stopTimer(UVTimerHandle *) {
    // trick to prevent uvloop from hanging
    _uvLoop->stop_flag = 1;
  }

  void close();

  uv_loop_t *getUVLoop() { return _uvLoop; }

protected:
  uv_loop_t *_uvLoop;
  size_t _activeHandles;
};
END_ASYNCIO_NAMESPACE;