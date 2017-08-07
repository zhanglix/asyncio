#pragma once

#include <mutex>
#include <uv.h>

#include <asyncio/common.hpp>

#include "loop_core.hpp"
#include "timer_handle.hpp"

BEGIN_ASYNCIO_NAMESPACE;
class UVTimerHandleImp;
class UVService {
public:
  UVService(uv_loop_t *uvLoop);
  virtual ~UVService() {}
  virtual void addHandle();
  virtual void subHandle();
  size_t activeHandlesCount() { return _activeHandles; }

  virtual void startTimer(UVTimerHandleImp *);
  virtual void stopTimer(UVTimerHandleImp *);
  uv_loop_t *getUVLoop() { return _uvLoop; }
  virtual void close();

protected:
  uv_loop_t *_uvLoop;
  size_t _activeHandles;
};

class UVTimerHandleImp : public TimerHandle {
public:
  UVTimerHandleImp(TimerCallback callback = nullptr, void *data = nullptr);
  void reset(TimerCallback callback, void *data);

  void setupTimer(); // promote protected startTimer() to public

protected:
  virtual bool executeTimer() override;

  TimerCallback _callback;
};

END_ASYNCIO_NAMESPACE;