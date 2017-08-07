#include "uv_timer_service.hpp"
#include "loop_exception.hpp"

USING_ASYNNCIO_NAMESPACE;

UVTimerHandle::UVTimerHandle(UVService *service) : UVHandle(service) {
  _uv_timer = new uv_timer_t;
  uvTimerInit();
}

UVTimerHandle::~UVTimerHandle() { close(); }

void UVTimerHandle::reset(uint64_t later, TimerCallback callback, void *data) {
  _later = later;
  UVHandle::reset(callback, data);
}

void UVTimerHandle::doStartTimer() {
  UVHandle::doStartTimer();
  uvTimerStart(_later);
}

void UVTimerHandle::doStopTimer() {
  uvTimerStop();
  UVHandle::doStopTimer();
}

void UVTimerHandle::uvTimerInit() {
  int err = uv_timer_init(getUVLoop(), _uv_timer);
  if (err != 0) {
    throw LoopException("uv_timer_init() failed.");
  }
  _uv_timer->data = this;
}

void UVTimerHandle::uvTimerStart(uint64_t later) {
  int err = uv_timer_start(
      _uv_timer,
      [](uv_timer_t *handle) { ((UVTimerHandle *)handle->data)->process(); },
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

// UVTimerService Following ...

UVTimerService::UVTimerService(uv_loop_t *uvLoop) : UVService(uvLoop) {}

TimerHandle *UVTimerService::callLater(uint64_t later, TimerCallback callback,
                                       void *data) {
  auto h = new UVTimerHandle(this);
  h->reset(later, callback, data);
  h->setupTimer();
  return (TimerHandle *)h;
}