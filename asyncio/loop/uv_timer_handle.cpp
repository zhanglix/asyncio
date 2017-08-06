#include "uv_timer_handle.hpp"

#include "loop_exception.hpp"

USING_ASYNNCIO_NAMESPACE;

bool UVTimerHandleBase::cancel() {
  if (_done) {
    return false;
  } else {
    completeTimer();
    return true;
  }
}

UVTimerHandle::UVTimerHandle(UVLoopCore *lc, TimerCallback callback, void *data)
    : UVTimerHandleBase(callback, data), _loop(lc) {
  _uv_timer = new uv_timer_t;
  _loop->addHandle();
  uvTimerInit();
}

void UVTimerHandle::runCallBack() {
  UVTimerHandleBase::runCallBack();
  completeTimer();
}

void UVTimerHandle::setupTimer(uint64_t later) {
  addRef();
  _done = false;
  uvTimerStart(later);
}

void UVTimerHandle::completeTimer() {
  uvTimerStop();
  _done = true;
  _loop->timerDone();
  subRef();
}

void UVTimerHandle::uvTimerInit() {
  int err = uv_timer_init(_loop->getUVLoop(), _uv_timer);
  if (err != 0) {
    throw LoopException("uv_timer_init() failed.");
  }
  _uv_timer->data = this;
}

void UVTimerHandle::uvTimerStart(uint64_t later) {
  int err = uv_timer_start(_uv_timer,
                           [](uv_timer_t *handle) {
                             ((UVTimerHandle *)handle->data)->runCallBack();
                           },
                           later, 0);
  if (err != 0) {
    throw LoopException("uv_timer_start() failed.");
  }
}

void UVTimerHandle::uvTimerStop() {
  int err = uv_timer_stop(_uv_timer);
  if (err != 0) {
    throw LoopException("uv_timer_stop() failed.");
  }
}

void UVTimerHandle::close() {
  uv_close((uv_handle_t *)_uv_timer,
           [](uv_handle_t *h) { delete (uv_timer_t *)h; });
}

UVTimerHandle::~UVTimerHandle() {
  _loop->subHandle();
  close();
}

// UVASyncTimerHandle following...

UVASyncTimerHandle::UVASyncTimerHandle(UVAsyncService *service,
                                       TimerCallback callback, void *data)
    : UVTimerHandleBase(callback, data), _service(service) {
  _service->addHandle();
}

UVASyncTimerHandle::~UVASyncTimerHandle() { _service->subHandle(); }

// size_t UVASyncTimerHandle::addRef() {
//   std::lock_guard<std::mutex> lock(_mutex);
//   return TimerHandle::addRef();
// }
// size_t UVASyncTimerHandle::subRef() {
//   std::lock_guard<std::mutex> lock(_mutex);
//   return TimerHandle::subRef();
// }

bool UVASyncTimerHandle::cancel() {
  if (completeUnlessDone()) {
    _service->timerCanceled(this);
    return true;
  }
  return false;
}

void UVASyncTimerHandle::completeTimer() { _done = true; }

bool UVASyncTimerHandle::completeUnlessDone() {
  std::lock_guard<std::mutex> lock(_mutex);
  if (!_done) {
    _done = true;
    return true;
  }
  return false;
}

void UVASyncTimerHandle::runCallBack() {
  if (completeUnlessDone()) {
    UVTimerHandleBase::runCallBack();
  }
}
