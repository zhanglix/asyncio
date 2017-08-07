#include "uv_timer_service.hpp"
#include "loop_exception.hpp"

USING_ASYNNCIO_NAMESPACE;

UVTimerHandle::UVTimerHandle(UVTimerService *service) : _service(service) {
  uvTimerInit();
}

void UVTimerHandle::doStartTimer() {
  uvTimerStart(_later);
  _service->startTimer(this);
}

void UVTimerHandle::doStopTimer() {
  uvTimerStop();
  _service->stopTimer(this);
}

void UVTimerHandle::uvTimerInit() {
  int err = uv_timer_init(_service->getUVLoop(), &_uv_timer);
  if (err != 0) {
    throw LoopException("uv_timer_init() failed.");
  }
  _uv_timer.data = this;
}

void UVTimerHandle::uvTimerStart(uint64_t later) {
  int err = uv_timer_start(
      &_uv_timer,
      [](uv_timer_t *handle) { ((UVTimerHandle *)handle->data)->process(); },
      later, 0);
  if (err != 0) {
    throw LoopException("uv_timer_start() failed.");
  }
}

void UVTimerHandle::uvTimerStop() {
  int err = uv_timer_stop(&_uv_timer);
  if (err != 0) {
    throw LoopException("uv_timer_stop() failed.");
  }
}

void UVTimerHandle::close() {
  uv_close((uv_handle_t *)&_uv_timer, nullptr);
  //  [](uv_handle_t *h) { delete (uv_timer_t *)h; });
}

UVTimerHandle::~UVTimerHandle() {
  _service->subHandle();
  close();
}

UVTimerService::UVTimerService(uv_loop_t *uvLoop)
    : _uvLoop(uvLoop), _activeHandles(0) {}
void UVTimerService::close() {
  if (activeHandlesCount() > 0) {
    throw LoopBusyError("UVTimerService Busy.");
  }
}