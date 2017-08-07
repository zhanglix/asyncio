#include "handle_base.hpp"

USING_ASYNNCIO_NAMESPACE;

HandleBase::HandleBase() { reset(); }
HandleBase::~HandleBase() {}

bool HandleBase::done() const { return _state == DONE; }

size_t HandleBase::refCount() const { return _refCount; }
size_t HandleBase::addRef() { return doAddRef(); }
size_t HandleBase::subRef() {
  if (doSubRef() == 0) {
    recycle();
    return 0;
  } else {
    return refCount();
  }
}

void HandleBase::recycle() { delete this; }

void HandleBase::reset() {
  _refCount = 1; // this reference is for end user;
  _state = READY;
}

size_t HandleBase::doAddRef() { return ++_refCount; }
size_t HandleBase::doSubRef() { return --_refCount; }

bool HandleBase::tryTransferState(State from, State to) {
  if (_state == from) {
    _state = to;
    return true;
  }
  return false;
}

void HandleBase::setDone() { _state = DONE; }

// BasicHandle Following ...

void BasicHandle::startTimer() {
  addRef();
  doStartTimer();
}

bool BasicHandle::cancel() {
  if (tryTransferState(READY, CANCELING) && cancelTimer()) {
    endTimer();
    return true; // to be removed
  }
  return false;
}

bool BasicHandle::process() {
  if (tryTransferState(READY, RUNNING) && executeTimer()) {
    endTimer();
    return true; // to be removed
  }
  return false;
}

void BasicHandle::endTimer() {
  setDone();
  doStopTimer();
  afterDone();
  subRef();
}

void BasicHandle::doStartTimer(){};
void BasicHandle::doStopTimer(){};
bool BasicHandle::executeTimer() { return true; }
bool BasicHandle::cancelTimer() { return true; };
void BasicHandle::afterDone() {}