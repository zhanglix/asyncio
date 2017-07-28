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
  TimerHandle() : TimerHandle(nullptr, nullptr) {}

  void *data() { return _data; }
  LoopCore *loopCore() { return _loopCore; }
  size_t refCount() { return _refCount; }

  void addRef() { ++_refCount; }
  void subRef() {
    if (--_refCount == 0) {
      _loopCore->recycle(this);
    }
  }

  void subRefThreadSafe() {
    (void)_loopCore->callSoonThreadSafe(subRefOnLoop, this);
  }

  void setLoopCore(LoopCore *lc) { _loopCore = lc; }
  void setData(void *data) { _data = data; }

  static void subRefOnLoop(TimerHandle *handle);

protected:
  LoopCore *_loopCore;
  void *_data;
  size_t _refCount;
};

END_ASYNCIO_NAMESPACE;