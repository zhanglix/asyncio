#include "uv_timer_handle.hpp"

#include "loop_exception.hpp"

USING_ASYNNCIO_NAMESPACE;

UVTimerHandle::UVTimerHandle(UVLoopCore *lc, TimerCallback callback, void *data)
    : UVTimerHandleBase(lc, callback, data) {
  _uv_timer = new uv_timer_t;
  _loop->addHandle();
  uvTimerInit();
}

void UVTimerHandle::runCallBack() {
  UVTimerHandleBase::runCallBack();
  completeTimer();
}

bool UVTimerHandle::completed() { return _completed; }

bool UVTimerHandle::cancel() {
  if (_completed) {
    return false;
  } else {
    completeTimer();
    return true;
  }
}

void UVTimerHandle::setupTimer(uint64_t later) {
  _completed = false;
  uvTimerStart(later);
  addRef();
}

void UVTimerHandle::completeTimer() {
  uvTimerStop();
  _completed = true;
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

bool UVASyncTimerHandle::completed() { return false; }
bool UVASyncTimerHandle::cancel() { return false; }