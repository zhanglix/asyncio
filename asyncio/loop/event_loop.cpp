#include "event_loop.hpp"

USING_ASYNNCIO_NAMESPACE;
void EventLoop::runUntilComplete(FutureBase *future) {
  while (!future->completed()) {
    _lc->runOneIteration();
  }
}
