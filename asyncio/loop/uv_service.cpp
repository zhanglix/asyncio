#include "uv_service.hpp"
#include "loop_exception.hpp"
USING_ASYNNCIO_NAMESPACE;

UVService::UVService(uv_loop_t *uvLoop) : _uvLoop(uvLoop), _activeHandles(0) {}

void UVService::close() {
  if (activeHandlesCount() > 0) {
    throw LoopBusyError("UVTimerService Busy.");
  }
}
void UVService::addHandle() { ++_activeHandles; }
void UVService::subHandle() { --_activeHandles; }

void UVService::startTimer(UVTimerHandleImp *handle) { addHandle(); }
void UVService::stopTimer(UVTimerHandleImp *handle) {
  // trick to prevent uvloop from hanging
  _uvLoop->stop_flag = 1;
}
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
