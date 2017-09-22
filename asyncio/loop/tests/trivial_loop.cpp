#include <sstream>

#include "../loop_exception.hpp"
#include "trivial_loop.hpp"

#define ENABLE_ASYNCIO_LOG
#include <asyncio/log.hpp>

using namespace std;

BEGIN_ASYNCIO_NAMESPACE;
void TrivialLoop::runOneIteration() {
  _now += 1;
  for (auto &&h : _timers) {
    h->processEntry();
    ASYNCIO_DEBUG("handle({})->refCount:({}) in runOneIteration.", (void *)h,
                  h->refCount());
    h->subRef();
  }
  _timers.clear();
}

void TrivialLoop::close() {
  if (_timers.size()) {
    stringstream ss;
    ss << "There are " << _timers.size() << " pending timers left!";
    throw LoopBusyError(ss.str());
  }
}

size_t TrivialLoop::activeHandlesCount() { return _timers.size(); }
uint64_t TrivialLoop::time() { return _now; }

TimerHandle *TrivialLoop::callSoon(TimerCallback callback, void *data) {
  ASYNCIO_DEBUG("call soon!");
  TrivialTimerHandle *handle = new TrivialTimerHandle(this, callback, data);
  handle->setupTimer();
  ASYNCIO_DEBUG("handle({})->refCount:({})", (void *)handle,
                handle->refCount());
  return handle;
}

TimerHandle *TrivialLoop::callSoonThreadSafe(TimerCallback callback,
                                             void *data) {
  return callSoon(callback, data);
}

TimerHandle *TrivialLoop::callLater(uint64_t milliseconds,
                                    TimerCallback callback, void *data) {
  return callSoon(callback, data);
}

void TrivialLoop::push(TrivialTimerHandle *handle) {
  handle->addRef();
  _timers.push_back(handle);
}

bool TrivialLoop::cancelTimer(TrivialTimerHandle *handle) {
  for (auto iter = _timers.begin(); iter != _timers.end(); iter++) {
    if (*iter == handle) {
      handle->subRef();
      _timers.erase(iter);
      break;
    }
  }
  return true;
}

END_ASYNCIO_NAMESPACE;