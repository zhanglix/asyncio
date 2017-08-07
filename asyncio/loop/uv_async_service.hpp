#pragma once

#include <mutex>
#include <uv.h>

#include <asyncio/common.hpp>

#include "extended_queue.hpp"
#include "loop_core.hpp"
#include "uv_service.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class UVASyncTimerHandle : public BasicHandleThreadSafe<UVTimerHandleBase> {
public:
  UVASyncTimerHandle(UVService *service)
      : BasicHandleThreadSafe<UVTimerHandleBase>(service) {}
};

class UVAsyncService : public UVService {
public:
  UVAsyncService(uv_loop_t *uvLoop);

  TimerHandle *callSoon(TimerCallback callback, void *data);

  void doStartTimer(UVTimerHandleBase *handle) override;
  void doStopTimer(UVTimerHandleBase *handle) override;

  void close() override;

  void addHandle() override;
  void subHandle() override;

protected:
  void setupService();
  void uvAsyncSend();
  void processTimers();

  void pushTimer(UVTimerHandleBase *handle);
  UVTimerHandleBase *popTimer();
  bool eraseTimer(UVTimerHandleBase *handle);

protected:
  uv_async_t *_uvAsync;
  std::mutex _mutex;
  ExtendedQueue<UVTimerHandleBase *> _queue;
};
END_ASYNCIO_NAMESPACE;