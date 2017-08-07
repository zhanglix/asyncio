#pragma once

#include <mutex>
#include <uv.h>

#include <asyncio/common.hpp>

#include "extended_queue.hpp"
#include "loop_core.hpp"
#include "uv_service.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class UVASyncTimerHandle : public BasicHandleThreadSafe<UVTimerHandleImp> {
public:
  UVASyncTimerHandle(UVService *);
  void doStartTimer() override;
  void doStopTimer() override;
  ~UVASyncTimerHandle();

protected:
  UVService *_service;
};

class UVAsyncService : public UVService {
public:
  UVAsyncService(uv_loop_t *uvLoop);

  TimerHandle *callSoon(TimerCallback callback, void *data);

  void startTimer(UVTimerHandleImp *handle) override;
  void stopTimer(UVTimerHandleImp *handle) override;

  void close() override;

  void addHandle() override;
  void subHandle() override;

protected:
  void setupService();
  void uvAsyncSend();
  void processTimers();

  void pushTimer(UVTimerHandleImp *handle);
  UVTimerHandleImp *popTimer();
  bool eraseTimer(UVTimerHandleImp *handle);

protected:
  uv_async_t *_uvAsync;
  std::mutex _mutex;
  ExtendedQueue<UVTimerHandleImp *> _queue;
};
END_ASYNCIO_NAMESPACE;