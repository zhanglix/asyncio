#pragma once

#include <asyncio/common.hpp>
#include <functional>

#include "handle_base.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class FutureBase : public BasicHandle {
public:
  virtual void release() = 0;
};

template <class R> class Future : public FutureBase {
public:
  using DoneCallback = std::function<void(Future<R> *)>;
  virtual R get() = 0;
  virtual void setDoneCallback(DoneCallback) = 0;
};

END_ASYNCIO_NAMESPACE;