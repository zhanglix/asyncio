#include "event_loop.hpp"
#include "uv_loop_core.hpp"

USING_ASYNNCIO_NAMESPACE;
EventLoop::EventLoop(LoopCore *lc, bool own) : _lc(lc), _owner(own) {
  if (_lc == nullptr) {
    _lc = new UVLoopCore();
    _owner = true;
  }
}

EventLoop::~EventLoop() {
  if (_owner) {
    delete _lc;
  }
}

void EventLoop::runUntilDone(FutureBase *future) {
  while (!future->done()) {
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
