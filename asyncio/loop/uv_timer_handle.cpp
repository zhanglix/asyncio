
#include "uv_timer_handle.hpp"

USING_ASYNNCIO_NAMESPACE;

UVTimerHandleImp::UVTimerHandleImp(TimerCallback callback, void *data) {
  reset(callback, data);
}
void UVTimerHandleImp::reset(TimerCallback callback, void *data) {
  _callback = callback;
  TimerHandle::reset(data);
}

void UVTimerHandleImp::processTimer() { process(); }

void UVTimerHandleImp::setupTimer() { startTimer(); }

bool UVTimerHandleImp::executeTimer() {
  (*_callback)(this);
  return true;
};
