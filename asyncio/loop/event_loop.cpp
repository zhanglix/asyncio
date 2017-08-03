#include "event_loop.hpp"

USING_ASYNNCIO_NAMESPACE;
EventLoop::EventLoop(LoopCore *lc) : _lc(lc) {}

void EventLoop::runUntilComplete(FutureBase *future) {
  while (!future->completed()) {
    _lc->runOneIteration();
  }
}

void EventLoop::runForever() {
  _stop = false;
  while (!_stop) {
    _lc->runOneIteration();
  }
}

void EventLoop::stop() { _stop = true; }
