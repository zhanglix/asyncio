
#include "uv_handle.hpp"
#include "uv_service.hpp"

USING_ASYNNCIO_NAMESPACE;

UVHandle::UVHandle(UVService *service, TimerCallback callback, void *data)
    : _service(service) {
  reset(callback, data);
}
void UVHandle::reset(TimerCallback callback, void *data) {
  _callback = callback;
  TimerHandle::reset(data);
}

void UVHandle::processTimer() { process(); }

void UVHandle::setupTimer() { startTimer(); }

void UVHandle::doStartTimer() { _service->startTimer(this); }

bool UVHandle::executeTimer() {
  (*_callback)(this);
  return true;
}

bool UVHandle::cancelTimer() { return _service->doCancelTimer(this); }

void UVHandle::doStopTimer() { _service->stopTimer(this); }

void UVHandle::recycle() { _service->recycleHandle(this); }

uv_loop_t *UVHandle::getUVLoop() { return _service->getUVLoop(); }