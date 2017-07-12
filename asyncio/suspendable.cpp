#include <experimental/coroutine>

#include "promise.hpp"
#include "suspendable.hpp"

#include <asyncio/config.hpp>

USING_ASYNNCIO_NAMESPACE;

void done_suspend::await_suspend(std::experimental::coroutine_handle<>) const
    noexcept {
  promise->set_done();
}