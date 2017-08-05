#include <sstream>

#include "loop_exception.hpp"
#include "uv_loop_core.hpp"
#include "uv_timer_handle.hpp"

using namespace std;
USING_ASYNNCIO_NAMESPACE;

UVLoopCore::UVLoopCore() : UVLoopCore(nullptr) {}

UVLoopCore::UVLoopCore(uv_loop_t *uvLoop) : _activeHandles(0) {
  _owner = (uvLoop == nullptr);
  if (_owner) {
    _loop = new uv_loop_t;
    uv_loop_init(_loop);
  } else {
    _loop = uvLoop;
  }
  _service = new UVAsyncService(_loop);
}

UVLoopCore::~UVLoopCore() { close(); }

void UVLoopCore::timerDone() { _loop->stop_flag = 1; }
void UVLoopCore::restoreLoop() { _loop->stop_flag = 0; }
void UVLoopCore::runOneIteration() {
  restoreLoop(); // tricks to fix uv_run() haning
  uv_run(_loop, UV_RUN_ONCE);
}

void UVLoopCore::close() { closeUVLoopT(); }

size_t UVLoopCore::activeHandlesCount() {
  size_t async_count = _service ? _service->activeHandlesCount() : 0;
  return _activeHandles + async_count;
}

uint64_t UVLoopCore::time() { return uv_now(_loop); }

TimerHandle *UVLoopCore::callSoon(TimerCallback callback, void *data) {
  return callLater(0, callback, data);
}

TimerHandle *UVLoopCore::callSoonThreadSafe(TimerCallback callback,
                                            void *data) {
  return _service->callSoon(callback, data);
}

TimerHandle *UVLoopCore::callLater(uint64_t milliseconds,
                                   TimerCallback callback, void *data) {
  auto handle = new UVTimerHandle(this, callback, data);
  handle->setupTimer(milliseconds);
  return handle;
}

void UVLoopCore::closeUVLoopT() {
  if (activeHandlesCount()) {
    throw LoopBusyError("UVLoopCore Busy.");
  }
  if (_service) {
    _service->close();
    delete _service;
    _service = nullptr;
  }
  if (_loop) {
    runOneIteration(); // clear pending closing callback;
  }
  if (_owner && _loop) {
    switch (int err = uv_loop_close(_loop)) {
    case 0:
      break;
    case UV_EBUSY:
      throw LoopBusyError();
      break;
    default:
      stringstream ss;
      ss << "uv_loop_close() failed. error: " << err;
      throw LoopException(ss.str());
    }
    delete _loop;
  }
  _loop = nullptr;
}