#pragma once

#include <asyncio/common.hpp>

#include "loop_core.hpp"
#include "timer_handle.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class UVTimerHandleBase : public TimerHandle {
public:
  UVTimerHandleBase(TimerCallback callback = nullptr, void *data = nullptr);
  void reset(TimerCallback callback, void *data);

  void processTimer(); // promote protected process() to public
  void setupTimer();   // promote protected startTimer() to public

protected:
  virtual bool executeTimer() override;

  TimerCallback _callback;
};

END_ASYNCIO_NAMESPACE;