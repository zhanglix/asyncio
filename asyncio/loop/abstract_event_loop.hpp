#include <chrono>
#include <functional>
#include <stdint.h>

#include <asyncio/common.hpp>
#include <asyncio/coroutine.hpp>

BEGIN_ASYNCIO_NAMESPACE;

class Future;
class Task;
class Handle;

class AbstractEventLoop {
public:
  virtual void runForever() = 0;
  virtual void runUntilComplete(Future *future) = 0;
  virtual bool isRunning() = 0;
  virtual void stop() = 0;
  virtual bool isClosed() = 0;
  virtual void closed() = 0;
  virtual void shutdownAsyncGens() = 0;
  virtual Handle *callLater(uint64_t milliseconds,
                            std::function<void()> call) = 0;
  virtual Handle *callSoonThreadSafe(std::function<void()> call) = 0;
  Handle *callSoon(std::function<void()> call) = 0;

  virtual uint64_t time() = 0;

  template <typename CoroType> Task<CoroType> *createTask(Coro &coro);

  template <typename CoroType> Task<CoroType> *createTask(Coro &&coro);
};

class Future {
public:
  virtual ready();
  virtual set_done()
};

class Handle {
public:
  virtual void release() = 0;
  virutal void cancel() = 0;
};


END_ASYNCIO_NAMESPACE;