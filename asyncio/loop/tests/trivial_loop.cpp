#include "trivial_loop.hpp"
#include "../loop_exception.hpp"

#include <sstream>

using namespace std;

BEGIN_ASYNCIO_NAMESPACE;
void TrivialLoop::runOneIteration() {
  _now += 1;
  for (auto &&h : _timers) {
    (*(h->callback))(h);
    h->setState(TimerHandle::State::FINISHED);
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
  TrivialTimerHandle *handle = new TrivialTimerHandle;
  handle->setLoopCore(this);
  handle->setData(data);
  handle->callback = callback;
  handle->addRef();
  _timers.push_back(handle);
  return handle;
}
TimerHandle *TrivialLoop::callSoonThreadSafe(TimerCallback callback,
                                             void *data) {
  return nullptr;
}
TimerHandle *TrivialLoop::callLater(uint64_t milliseconds,
                                    TimerCallback callback, void *data) {
  return nullptr;
}
bool TrivialLoop::cancelTimer(TimerHandle *handle) {
  for (auto iter = _timers.begin(); iter != _timers.end(); iter++) {
    if (*iter == handle) {
      handle->setState(TimerHandle::State::CANCELED);
      handle->subRef();
      _timers.erase(iter);
      return true;
    }
  }
  return false;
}

void TrivialLoop::recycleTimerHandle(TimerHandle *handle) { delete handle; }

END_ASYNCIO_NAMESPACE;