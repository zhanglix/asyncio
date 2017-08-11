#pragma once

#include "loop_awaitable.hpp"
#include "../coro/coro.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class Sleep : public LoopAWaitable<void> {
public:
  Sleep(EventLoop *loop, uint64_t ms) : LoopAWaitable(loop), _ms(ms) {}
  void run() override;

private:
  uint64_t _ms;
};

template<class A=DefaultAllocator>
coro<void, A> sleep(EventLoop *loop, uint64_t ms) {
   co_await Sleep(loop, ms);
   co_return;
}

END_ASYNCIO_NAMESPACE;