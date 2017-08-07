#pragma once

#include <mutex>

#include <asyncio/common.hpp>

#include "timer_handle.hpp"
#include "uv_loop_core.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class UVTimerHandleBase : public TimerHandle {
public:
  UVTimerHandleBase(TimerCallback callback, void *data)
      : TimerHandle(data), _callback(callback), _done(false) {}
  virtual void runCallBack() { (*_callback)(this); }
  bool done() const override { return _done; }
  bool cancel() override;
  virtual void completeTimer() = 0;

protected:
  TimerCallback _callback;
  bool _done;
};

// class UVASyncTimerHandle : public UVTimerHandleBase {
// public:
//   UVASyncTimerHandle(UVAsyncService *service, TimerCallback callback,
//                      void *data);
//   ~UVASyncTimerHandle();

//   // size_t addRef() override;
//   // size_t subRef() override;

//   void runCallBack() override;

//   bool cancel() override;
//   void completeTimer() override;
//   bool completeUnlessDone();

// protected:
//   UVAsyncService *_service;
//   std::mutex _mutex;
// };

END_ASYNCIO_NAMESPACE;