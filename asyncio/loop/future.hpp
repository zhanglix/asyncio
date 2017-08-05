#pragma once

#include <asyncio/common.hpp>
#include <functional>

BEGIN_ASYNCIO_NAMESPACE;

class FutureBase {
public:
  virtual ~FutureBase() {}
  virtual bool done() const = 0;
  virtual bool cancel() = 0;
  virtual void release() = 0;
};

template <class R> class Future : public FutureBase {
public:
  using DoneCallback = std::function<void(Future<R> *)>;
  virtual ~Future() {}
  virtual R get() = 0;
  virtual void setDoneCallback(DoneCallback) = 0;
};

END_ASYNCIO_NAMESPACE;