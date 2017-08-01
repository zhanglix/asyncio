
#include <asyncio/common.hpp>

#include "loop_core.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class UVLoopCore : public LoopCore {
public:
  UVLoopCore();
  UVLoopCore(uv_loop_t *uvLoop);

  virtual ~UVLoopCore() {}
  virtual void runOneIteration() override;
  virtual void close() override;
  virtual size_t activeHandlesCount() override;
  virtual uint64_t time() override;
  virtual TimerHandle *callSoon(TimerCallback callback, void *data) override;
  virtual TimerHandle *callSoonThreadSafe(TimerCallback callback,
                                          void *data) override;
  virtual TimerHandle *callLater(uint64_t milliseconds, TimerCallback callback,
                                 void *data) override;
  virtual bool cancelTimer(TimerHandle *handle) override;
  virtual void recycleTimerHandle(TimerHandle *handle) override;

private:
  uv_loop_t *_loop;
  bool _owner;
};
END_ASYNCIO_NAMESPACE;