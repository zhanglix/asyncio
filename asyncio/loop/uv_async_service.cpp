#include "uv_async_service.hpp"
#include "loop_exception.hpp"
#include "uv_timer_handle.hpp"

using namespace std;
USING_ASYNNCIO_NAMESPACE;

UVAsyncService::UVAsyncService(uv_loop_t *uvLoop)
    : _uvLoop(uvLoop), _activeHandles(0) {
  setupService();
}

size_t UVAsyncService::addHandle() {
  lock_guard<mutex> lock(_mutex);
  return ++_activeHandles;
}

size_t UVAsyncService::subHandle() {
  lock_guard<mutex> lock(_mutex);
  return --_activeHandles;
}

void UVAsyncService::setupService() {
  uv_async_t *uvAsync = new uv_async_t;
  int err = uv_async_init(_uvLoop, uvAsync, [](uv_async_t *h) {
    auto service = (UVAsyncService *)(h->data);
    service->callTimers();
  });
  if (err != 0) {
    delete uvAsync;
    throw LoopException("uv_async_init() failed.");
  }
  _uvAsync = uvAsync;
  _uvAsync->data = this;
}

void UVAsyncService::uvAsyncSend() {
  int err = uv_async_send(_uvAsync);
  if (err != 0) {
    throw LoopException("uv_async_send() failed.");
  }
}

UVASyncTimerHandle *UVAsyncService::callSoon(TimerCallback callback,
                                             void *data) {
  auto handle = new UVASyncTimerHandle(this, callback, data);
  handle->addRef();
  pushTimer(handle);
  return handle;
}

void UVAsyncService::pushTimer(UVASyncTimerHandle *handle) {
  lock_guard<mutex> lock(_mutex);
  _queue.push(handle);
}

UVASyncTimerHandle *UVAsyncService::popTimer() {
  lock_guard<mutex> lock(_mutex);
  if (!_queue.empty()) {
    UVASyncTimerHandle *handle = _queue.front();
    _queue.pop();
    return handle;
  } else {
    return nullptr;
  }
}

void UVAsyncService::callTimers() {
  while (true) {
    UVASyncTimerHandle *handle = popTimer();
    if (handle) {
      handle->runCallBack();
      handle->subRef();
    } else {
      return;
    }
  };
}

void UVAsyncService::close() {
  if (_activeHandles > 0) {
    throw LoopBusyError("UVAsyncService busy.");
  }

  uv_close((uv_handle_t *)(_uvAsync), [](uv_handle_t *h) {
    auto handle = (uv_async_t *)h;
    delete handle;
  });
}

void UVAsyncService::tryActive() {
  if (_activeHandles > 0) {
    uv_ref((uv_handle_t *)_uvAsync);
    uvAsyncSend();
  } else {
    uv_unref((uv_handle_t *)_uvAsync);
  }
}
