#pragma once

#include <cstddef>
#include <cstdint>

#include <asyncio/common.hpp>

#include "loop_core.hpp"

BEGIN_ASYNCIO_NAMESPACE;
class TimerHandle {
public:
  TimerHandle(LoopCore *lc, void *data)
      : _loopCore(lc), _data(data), _refCount(1) {}
  virtual ~TimerHandle() {}

  LoopCore *loopCore() const { return _loopCore; }
  void setLoopCore(LoopCore *lc) { _loopCore = lc; }

  void *data() const { return _data; }
  void setData(void *data) { _data = data; }

  virtual size_t refCount() const { return _refCount; }
  virtual size_t addRef() { return ++_refCount; }
  virtual size_t subRef() {
    size_t refs = --_refCount;
    if (refs == 0) {
      delete this;
    }
    return refs;
  }

  virtual bool completed() = 0;
  virtual bool cancel() = 0;

  virtual void reset(void *data = nullptr) {
    _data = data;
    _refCount = 1;
  }

protected:
  LoopCore *_loopCore;
  void *_data;
  size_t _refCount;
};

// DefautTimerHandle To be deleted!
class DefaultTimerHandle : public TimerHandle {
public:
  enum State { READY = 0, FINISHED, CANCELED };

  DefaultTimerHandle(LoopCore *lc, void *data)
      : TimerHandle(lc, data), _state(READY) {}
  DefaultTimerHandle() : DefaultTimerHandle(nullptr, nullptr) {}

  size_t subRef() {
    if (--_refCount == 0) {
      _loopCore->recycleTimerHandle(this);
      return 0;
    } else {
      return _refCount;
    }
  }

  void subRefThreadSafe() {
    (void)_loopCore->callSoonThreadSafe(subRefOnLoop, this);
  }

  bool cancel() { return _loopCore->cancelTimer(this); }
  bool completed() { return _state != State::READY; }
  State getState() const { return _state; }
  void setState(DefaultTimerHandle::State state) { _state = state; }

  void reset(void *data = nullptr) {
    TimerHandle::reset(data);
    _state = State::READY;
  }

  static void subRefOnLoop(TimerHandle *handle);

protected:
  State _state;
};

END_ASYNCIO_NAMESPACE;