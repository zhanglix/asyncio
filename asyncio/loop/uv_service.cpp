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

void UVService::startTimer(UVHandle *handle) {
  addHandle();
  doStartTimer(handle);
}

void UVService::stopTimer(UVHandle *handle) {
  doStopTimer(handle);
  // trick to prevent uvloop from hanging
  _uvLoop->stop_flag = 1;
}

void UVService::recycleHandle(UVHandle *handle) {
  subHandle();
  delete handle;
}
