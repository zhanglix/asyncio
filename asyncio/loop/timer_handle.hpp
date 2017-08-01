#pragma once

#include <cstddef>
#include <cstdint>

#include <asyncio/common.hpp>

#include "loop_core.hpp"

BEGIN_ASYNCIO_NAMESPACE;
class TimerHandle {
public:
  TimerHandle(void *data) : _data(data), _refCount(1) {}
  virtual ~TimerHandle() {}

  void *data() const { return _data; }
  void setData(void *data) { _data = data; }

  virtual LoopCore *loopCore() const = 0;

  virtual size_t refCount() const { return _refCount; }
  virtual size_t addRef() { return ++_refCount; }
  virtual size_t subRef() {
    size_t refs = --_refCount;
    if (refs == 0) {
      release();
    }
    return refs;
  }
  virtual void release() { delete this; }

  virtual bool completed() = 0;
  virtual bool cancel() = 0;

  virtual void reset(void *data = nullptr) {
    _data = data;
    _refCount = 1;
  }

protected:
  void *_data;
  size_t _refCount;
};

END_ASYNCIO_NAMESPACE;
