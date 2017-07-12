#include <experimental/coroutine>

#include "promise.hpp"
#include "suspendable.hpp"

#include <asyncio/config.hpp>

USING_ASYNNCIO_NAMESPACE;
using namespace std::experimental;

void done_suspend::await_suspend(coroutine_handle<>) const noexcept {
  promise->set_done();
}