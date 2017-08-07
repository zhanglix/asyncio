#pragma once

#include <mutex>
#include <uv.h>

#include <asyncio/common.hpp>

#include "extended_queue.hpp"
#include "loop_core.hpp"
#include "uv_service.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class UVASyncTimerHandle : public BasicHandleThreadSafe<UVHandle> {
public:
  UVASyncTimerHandle(UVService *service)
      : BasicHandleThreadSafe<UVHandle>(service) {}
};

class UVAsyncService : public UVService {
public:
  UVAsyncService(uv_loop_t *uvLoop);

  TimerHandle *callSoon(TimerCallback callback, void *data);

  void doStartTimer(UVHandle *handle) override;
  void doStopTimer(UVHandle *handle) override;

  void close() override;

  void addHandle() override;
  void subHandle() override;

protected:
  void setupService();
  void uvAsyncSend();
  void processTimers();

  void pushTimer(UVHandle *handle);
  UVHandle *popTimer();
  bool eraseTimer(UVHandle *handle);

protected:
  uv_async_t *_uvAsync;
  std::mutex _mutex;
  ExtendedQueue<UVHandle *> _queue;
};
END_ASYNCIO_NAMESPACE;