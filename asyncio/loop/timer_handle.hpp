#pragma once

#include <cstddef>
#include <cstdint>

#include <asyncio/common.hpp>

#include "abstract_handle.hpp"
#include "loop_core.hpp"

BEGIN_ASYNCIO_NAMESPACE;
class TimerHandle : public BasicHandle {
public:
  TimerHandle(void *data) : _data(data) {}
  void *data() const { return _data; }
  void setData(void *data) { _data = data; }
  void reset(void *data = nullptr) {
    _data = data;
    BasicHandle::reset();
  }

protected:
  void *_data;
};

END_ASYNCIO_NAMESPACE;
