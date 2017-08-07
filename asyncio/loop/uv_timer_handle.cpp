
#include "uv_timer_handle.hpp"

USING_ASYNNCIO_NAMESPACE;

UVTimerHandleBase::UVTimerHandleBase(TimerCallback callback, void *data) {
  reset(callback, data);
}
void UVTimerHandleBase::reset(TimerCallback callback, void *data) {
  _callback = callback;
  TimerHandle::reset(data);
}

void UVTimerHandleBase::processTimer() { process(); }

void UVTimerHandleBase::setupTimer() { startTimer(); }

bool UVTimerHandleBase::executeTimer() {
  (*_callback)(this);
  return true;
};
