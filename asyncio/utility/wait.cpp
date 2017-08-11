#include "wait.hpp"

BEGIN_ASYNCIO_NAMESPACE;
FutureCoGen::FutureCoGen(std::vector<FutureBase *> &futs)
    : _next(0), _futs(futs), _awaitables(_futs.size()) {
  for (auto &&fut : _futs) {
    fut->addRef();
    fut->setDoneCallback(
        [&](FutureBase *h) { _awaitables[_next++].resume(h); });
  }
}
FutureCoGen::~FutureCoGen() {
  for (auto &&fut : _futs) {
    if (!fut->done()) {
      fut->setDoneCallback([](FutureBase *) {});
    }
    fut->release();
  }
}

bool FutureCoGen::hasWaitable(size_t next) { return next < _futs.size(); }

AWaitable<FutureBase *> &FutureCoGen::waitable(size_t pos) {
  return _awaitables[pos];
}

FutureBase *FutureCoGen::future(size_t pos) { return _futs[pos]; }

END_ASYNCIO_NAMESPACE;