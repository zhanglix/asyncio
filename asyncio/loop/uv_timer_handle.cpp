#include "uv_timer_handle.hpp"

USING_ASYNNCIO_NAMESPACE;

bool UVTimerHandle::completed() { return false; }
bool UVTimerHandle::cancel() { return false; }

bool UVASyncTimerHandle::completed() { return false; }
bool UVASyncTimerHandle::cancel() { return false; }