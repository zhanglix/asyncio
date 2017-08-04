#include "asleep.hpp"

BEGIN_ASYNCIO_NAMESPACE;

void ASleep::run() {
  loop()->callLater(_ms, [&] { this->resume(); })->release();
}

END_ASYNCIO_NAMESPACE;