#pragma once

#include <cstddef>
#include <cstdint>

#include <asyncio/common.hpp>

#include "loop_core.hpp"

BEGIN_ASYNCIO_NAMESPACE;
class TimerHandle {
public:
  enum State { READY = 0, FINISHED, CANCELED };
  TimerHandle(LoopCore *lc, void *data)
      : _loopCore(lc), _data(data), _refCount(1), _state(READY) {}
  TimerHandle() : TimerHandle(nullptr, nullptr) {}

  void *data() const { return _data; }
  LoopCore *loopCore() const { return _loopCore; }
  size_t refCount() const { return _refCount; }

  void addRef() { ++_refCount; }
  void subRef() {
    if (--_refCount == 0) {
      _loopCore->recycleTimerHandle(this);
    }
  }

  void subRefThreadSafe() {
    (void)_loopCore->callSoonThreadSafe(subRefOnLoop, this);
  }

  bool cancel() { return _loopCore->cancelTimer(this); }
  bool completed() { return _state != State::READY; }
  void setLoopCore(LoopCore *lc) { _loopCore = lc; }
  void setData(void *data) { _data = data; }
  State getState() const { return _state; }
  void setState(TimerHandle::State state) { _state = state; }

  void reset(void *data = nullptr) {
    _data = data;
    _refCount = 1;
    _state = State::READY;
  }

  static void subRefOnLoop(TimerHandle *handle);

protected:
  LoopCore *_loopCore;
  void *_data;
  size_t _refCount;
  State _state;
};

END_ASYNCIO_NAMESPACE;