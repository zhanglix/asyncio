#pragma once

#include "loop_awaitable.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class Sleep : public LoopAWaitable<void> {
public:
  Sleep(EventLoop *loop, uint64_t ms) : LoopAWaitable(loop), _ms(ms) {}
  void run() override;

private:
  uint64_t _ms;
};

using sleep = Sleep;

END_ASYNCIO_NAMESPACE;