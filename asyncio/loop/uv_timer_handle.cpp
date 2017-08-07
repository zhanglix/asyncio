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
