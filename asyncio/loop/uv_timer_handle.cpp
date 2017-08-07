
#include "uv_timer_handle.hpp"
#include "uv_service.hpp"

USING_ASYNNCIO_NAMESPACE;

UVTimerHandleBase::UVTimerHandleBase(UVService *service, TimerCallback callback,
                                     void *data)
    : _service(service) {
  reset(callback, data);
}
void UVTimerHandleBase::reset(TimerCallback callback, void *data) {
  _callback = callback;
  TimerHandle::reset(data);
}

void UVTimerHandleBase::processTimer() { process(); }

void UVTimerHandleBase::setupTimer() { startTimer(); }

void UVTimerHandleBase::doStartTimer() { _service->startTimer(this); }

bool UVTimerHandleBase::executeTimer() {
  (*_callback)(this);
  return true;
};

void UVTimerHandleBase::doStopTimer() { _service->stopTimer(this); }

void UVTimerHandleBase::recycle() { _service->recycleHandle(this); }
