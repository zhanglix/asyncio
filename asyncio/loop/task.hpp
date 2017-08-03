#pragma once

#include <asyncio/common.hpp>

#include "future.hpp"
#include "timer_future.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <class R> class Task : public Future<R> {
public:
  virtual ~Task() {}

  bool completed() override { return true; }
  bool cancel() override { return true; }
  void release() override {}
  R get() override { return R(); }
};

END_ASYNCIO_NAMESPACE;
