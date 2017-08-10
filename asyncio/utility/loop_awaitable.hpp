#pragma once

#include "../common.hpp"
#include "../coro/coro.hpp"
#include "../coro/utility.hpp"
#include "../loop/event_loop.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <class T> class LoopAWaitable : public AWaitable<T> {
public:
  LoopAWaitable(EventLoop *loop) : _loop(loop) {}
  EventLoop *loop() const { return _loop; }

private:
  EventLoop *_loop;
};

END_ASYNCIO_NAMESPACE;