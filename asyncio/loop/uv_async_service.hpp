#pragma once

#include <mutex>
#include <queue>
#include <uv.h>

#include <asyncio/common.hpp>

#include "loop_core.hpp"

BEGIN_ASYNCIO_NAMESPACE;
class UVASyncTimerHandle;
class UVAsyncService {
public:
  UVAsyncService(uv_loop_t *uvLoop);

  size_t addHandle();
  size_t subHandle();
  size_t activeHandlesCount() { return _activeHandles; }

  void setupService();
  void uvAsyncSend();

  UVASyncTimerHandle *callSoon(TimerCallback callback, void *data);

  void pushTimer(UVASyncTimerHandle *handle);
  UVASyncTimerHandle *popTimer();
  void callTimers();

  void close();

protected:
  uv_loop_t *_uvLoop;
  uv_async_t *_uvAsync;
  size_t _activeHandles;
  std::mutex _mutex;
  std::queue<UVASyncTimerHandle *> _queue;
};
END_ASYNCIO_NAMESPACE;