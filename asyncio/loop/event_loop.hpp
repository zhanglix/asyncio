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
  virtual ~EventLoop();

  virtual uint64_t time() { return _lc->time(); }

  virtual void runUntilDone(FutureBase *future);
  virtual void runForever();
  virtual void stop();

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

    auto timerFuture = new FutureType(_lc, milliseconds, call);
    timerFuture->setupTimer();
    return (Future<R> *)timerFuture;
  }

  template <bool threadSafe, class CoroType>
  auto taskOnTimer(uint64_t milliseconds, CoroType &&co) {
    using C = std::remove_reference_t<CoroType>;
    using R = typename C::ReturnType;
    using FutureType = Task<C, R, threadSafe>;

    auto timerFuture =
        new FutureType(_lc, milliseconds, std::forward<CoroType>(co));
    timerFuture->setupTimer();
    return (Future<R> *)timerFuture;
  }

private:
  LoopCore *_lc;
  bool _owner;
  bool _stop;
};

END_ASYNCIO_NAMESPACE;