#include "timer_handle.hpp"

USING_ASYNNCIO_NAMESPACE;
TimerHandle::TimerHandle(void *data) : _data(data) {}
void *TimerHandle::data() const { return _data; }
void TimerHandle::setData(void *data) { _data = data; }
void TimerHandle::reset(void *data) {
  _data = data;
  BasicHandle::reset();
}