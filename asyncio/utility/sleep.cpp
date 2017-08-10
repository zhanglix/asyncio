#include "sleep.hpp"

BEGIN_ASYNCIO_NAMESPACE;

void Sleep::run() {
  loop()->callLater(_ms, [&] { this->resume(); })->release();
}

END_ASYNCIO_NAMESPACE;