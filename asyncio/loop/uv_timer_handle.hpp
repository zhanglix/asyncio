#pragma once

#include <asyncio/common.hpp>

#include "timer_handle.hpp"
#include "uv_loop_core.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class UVTimerHandleBase : public TimerHandle {
public:
  UVTimerHandleBase(TimerCallback callback, void *data)
      : TimerHandle(data), _callback(callback) {}
  virtual void runCallBack() { (*_callback)(this); }

protected:
  TimerCallback _callback;
};

class UVTimerHandle : public UVTimerHandleBase {
public:
  UVTimerHandle(UVLoopCore *lc, TimerCallback callback, void *data);
  ~UVTimerHandle();
  bool completed() override;
  bool cancel() override;
  void runCallBack() override;

  void uvTimerInit();
  void uvTimerStart(uint64_t later);
  void uvTimerStop();
  void close();

  void setupTimer(uint64_t later);
  void completeTimer();

protected:
  UVLoopCore *_loop;
  uv_timer_t *_uv_timer;
  bool _completed;
};

class UVASyncTimerHandle : public UVTimerHandleBase {
public:
  UVASyncTimerHandle(TimerCallback callback, void *data)
      : UVTimerHandleBase(callback, data) {}

  bool completed() override;
  bool cancel() override;
};

END_ASYNCIO_NAMESPACE;