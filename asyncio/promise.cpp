
#include "promise.hpp"
#include "suspendable.hpp"

USING_ASYNNCIO_NAMESPACE;

done_suspend promise_base::final_suspend() { return done_suspend(this); }
