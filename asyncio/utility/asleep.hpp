#pragma once

#include "../common.hpp"
#include "../coro/coro.hpp"
#include "../coro/utility.hpp"
#include "../loop/event_loop.hpp"

BEGIN_ASYNCIO_NAMESPACE;
template <class A = DefaultAllocator>
coro<void, A> asleep(uint64_t millisecond, EventLoop *loop) {
  AWaitable<void> awaitable;
  loop->callLater(millisecond, [&] { awaitable.resume(); })->release();
  co_await awaitable;
}
END_ASYNCIO_NAMESPACE;