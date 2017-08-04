#pragma once

#include "loop_awaitable.hpp"

BEGIN_ASYNCIO_NAMESPACE;

class ASleep : public LoopAWaitable<void> {
public:
  ASleep(EventLoop *loop, uint64_t ms) : LoopAWaitable(loop), _ms(ms) {}
  void run() override;

private:
  uint64_t _ms;
};

using asleep = ASleep;

END_ASYNCIO_NAMESPACE;