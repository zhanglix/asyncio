#pragma once
#include <chrono>
#include <functional>
#include <stdint.h>
#include <type_traits>
#include <utility>

#include <asyncio/common.hpp>

#include "future.hpp"
#include "loop_core.hpp"
#include "task.hpp"
#include "timer_future.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class EventLoop {
public:
  EventLoop(LoopCore *lc = nullptr, bool own = true);
  ~EventLoop();

  void runUntilDone(FutureBase *future);
  void runForever();
  void stop();

  template <class F, class... Args> auto callSoon(F &&f, Args &&... args) {
    return callOnTimer<false, F, Args...>(0, std::forward<F>(f),
                                          std::forward<Args>(args)...);
  }

  template <class F, class... Args>
  auto callSoonThreadSafe(F &&f, Args &&... args) {
    return callOnTimer<true, F, Args...>(0, std::forward<F>(f),
                                         std::forward<Args>(args)...);
  }

  template <class F, class... Args>
  auto callLater(uint64_t milliseconds, F &&f, Args &&... args) {
    return callOnTimer<false, F, Args...>(milliseconds, std::forward<F>(f),
                                          std::forward<Args>(args)...);
  }

  template <class CoroType> auto createTask(CoroType &&co) {
    return taskOnTimer<false, CoroType>(0, std::forward<CoroType>(co));
  }

  template <class CoroType>
  auto createTaskLater(uint64_t milliseconds, CoroType &&co) {
    return taskOnTimer<false, CoroType>(milliseconds,
                                        std::forward<CoroType>(co));
  }

  template <class CoroType> auto createTaskThreadSafe(CoroType &&co) {
    return taskOnTimer<true, CoroType>(0, std::forward<CoroType>(co));
  }

protected:
  template <bool threadSafe, class F, class... Args>
  auto callOnTimer(uint64_t milliseconds, F &&f, Args &&... args) {
    using R = typename std::result_of<F(Args...)>::type;
    using FutureType = TimerFuture<R, threadSafe>;
    std::function<R(void)> call =
        std::bind(std::forward<F>(f), std::forward<Args>(args)...);

    auto timerFuture = new FutureType(call);
    setupFuture<threadSafe, FutureType>(timerFuture, milliseconds);
    return (Future<R> *)timerFuture;
  }

  template <bool threadSafe, class CoroType>
  auto taskOnTimer(uint64_t milliseconds, CoroType &&co) {
    using C = std::remove_reference_t<CoroType>;
    using R = typename C::ReturnType;
    using FutureType = Task<C, R, threadSafe>;

    auto timerFuture = new FutureType(std::forward<CoroType>(co));
    setupFuture<threadSafe, FutureType>(timerFuture, milliseconds);
    return (Future<R> *)timerFuture;
  }

protected:
  template <bool threadSafe, class Fut>
  std::enable_if_t<threadSafe> setupFuture(Fut *fut, uint64_t) {
    fut->addRef();
    auto handle = _lc->callSoonThreadSafe(fut->processEntry, fut);
    fut->setHandle(handle);
  }

  template <bool threadSafe, class Fut>
  std::enable_if_t<!threadSafe> setupFuture(Fut *fut, uint64_t ms) {
    fut->addRef();
    auto handle = ms > 0 ? _lc->callLater(ms, fut->processEntry, fut)
                         : _lc->callSoon(fut->processEntry, fut);
    fut->setHandle(handle);
  }

private:
  LoopCore *_lc;
  bool _owner;
  bool _stop;
};

END_ASYNCIO_NAMESPACE;