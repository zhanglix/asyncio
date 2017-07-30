#pragma once

#include <asyncio/common.hpp>
// #include <future>
// #include <utility>

BEGIN_ASYNCIO_NAMESPACE;

class FutureBase {
public:
  virtual ~FutureBase() {}
  virtual bool completed() = 0;
  virtual bool cancel() = 0;
  virtual void release() = 0;
};

template <class R> class Future : public FutureBase {
public:
  virtual ~Future() {}
  virtual R get() = 0;
};

END_ASYNCIO_NAMESPACE;