#include <chrono>
#include <functional>
#include <stdint.h>

namespace asyncio {
class Future;
class Handle;
class AWaitable;

template <typename LoopType, typename LoopAllocator> class AbstractEventLoop {
public:
  virtual void runForever() = 0;
  virtual void runUntilComplete(Future *future) = 0;
  virtual bool isRunning() = 0;
  virtual void stop() = 0;
  virtual bool isClosed() = 0;
  virtual void closed() = 0;
  virtual void shutdownAsyncGens() = 0;
  virtual Handle callSoon(std::function<void()> call) = 0;
  virtual Handle callSoonThreadSafe(std::function<void()> call) = 0;
  virtual Handle callLater(uint64_t milliseconds,
                           std::function<void()> call) = 0;
  virtual uint64_t time() = 0;
  virtual Future *create_future();
  virtual Task *createTask(std::function<AWaitable()>)
};
}