#pragma once

#include <mutex>

#include <asyncio/common.hpp>

#include "timer_handle.hpp"
#include "uv_loop_core.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class UVTimerHandleBase : public TimerHandle {
public:
  UVTimerHandleBase(TimerCallback callback, void *data)
      : TimerHandle(data), _callback(callback), _completed(false) {}
  virtual void runCallBack() { (*_callback)(this); }
  bool completed() override { return _completed; }
  bool cancel() override;
  virtual void completeTimer() = 0;

protected:
  TimerCallback _callback;
  bool _completed;
};

class UVTimerHandle : public UVTimerHandleBase {
public:
  UVTimerHandle(UVLoopCore *lc, TimerCallback callback, void *data);
  ~UVTimerHandle();
  void runCallBack() override;

  void uvTimerInit();
  void uvTimerStart(uint64_t later);
  void uvTimerStop();
  void close();

  void setupTimer(uint64_t later);
  void completeTimer() override;

protected:
  UVLoopCore *_loop;
  uv_timer_t *_uv_timer;
};

class UVASyncTimerHandle : public UVTimerHandleBase {
public:
  UVASyncTimerHandle(UVAsyncService *service, TimerCallback callback,
                     void *data);
  ~UVASyncTimerHandle();

  size_t addRef() override;
  size_t subRef() override;

  void runCallBack() override;

  bool cancel() override;
  void completeTimer() override;
  bool completeUnlessCanceled();

protected:
  UVAsyncService *_service;
  std::mutex _mutex;
};

END_ASYNCIO_NAMESPACE;