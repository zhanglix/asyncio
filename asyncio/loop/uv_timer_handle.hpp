#pragma once

#include <asyncio/common.hpp>

#include "timer_handle.hpp"
#include "uv_loop_core.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class UVTimerHandleBase : public TimerHandle {
public:
  UVTimerHandleBase(UVLoopCore *lc, void *data)
      : TimerHandle(data), _loop(lc) {}
  LoopCore *loopCore() const override { return _loop; }

protected:
  UVLoopCore *_loop;
};

class UVTimerHandle : public UVTimerHandleBase {
public:
  UVTimerHandle(UVLoopCore *lc, void *data) : UVTimerHandleBase(lc, data) {}

  bool completed() override;
  bool cancel() override;
};

class UVASyncTimerHandle : public UVTimerHandleBase {
public:
  UVASyncTimerHandle(UVLoopCore *lc, void *data)
      : UVTimerHandleBase(lc, data) {}

  bool completed() override;
  bool cancel() override;
};

END_ASYNCIO_NAMESPACE;