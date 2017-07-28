
#include "timer_handle.hpp"
#include "loop_core.hpp"

BEGIN_ASYNCIO_NAMESPACE;

void TimerHandle::subRefOnLoop(TimerHandle *handle) {
  auto handleToSubRef = (TimerHandle *)handle->data();
  handleToSubRef->subRef();
  handle->subRef();
}
END_ASYNCIO_NAMESPACE;