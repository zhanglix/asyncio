#pragma once

#include <uv.h>

#include <asyncio/common.hpp>

#include "loop_core.hpp"
#include "uv_async_service.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class UVLoopCore : public LoopCore {
public:
  UVLoopCore();
  UVLoopCore(uv_loop_t *uvLoop);

  virtual ~UVLoopCore();
  virtual void runOneIteration() override;
  virtual void close() override;
  virtual size_t activeHandlesCount() override;
  virtual uint64_t time() override;
  virtual TimerHandle *callSoon(TimerCallback callback, void *data) override;
  virtual TimerHandle *callSoonThreadSafe(TimerCallback callback,
                                          void *data) override;
  virtual TimerHandle *callLater(uint64_t milliseconds, TimerCallback callback,
                                 void *data) override;

  uv_loop_t *getUVLoop() const { return _loop; }
  void timerCompleted();
  void restoreLoop();

  size_t addHandle() { return ++_activeHandles; }
  size_t subHandle() { return --_activeHandles; }

private:
  void closeUVLoopT();
  uv_loop_t *_loop;
  bool _owner;
  size_t _activeHandles;
  UVAsyncService *_service;
};
END_ASYNCIO_NAMESPACE;