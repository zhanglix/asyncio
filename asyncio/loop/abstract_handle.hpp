#pragma once
#include <asyncio/common.hpp>

BEGIN_ASYNCIO_NAMESPACE;

class HandleBase {
public:
  HandleBase() { reset(); }
  virtual ~HandleBase() {}

  virtual bool done() const { return _state == DONE; }
  virtual bool cancel() = 0;

  size_t refCount() const { return _refCount; }
  size_t addRef() { return doAddRef(); }
  size_t subRef() {
    if (doSubRef() == 0) {
      recycle();
      return 0;
    } else {
      return refCount();
    }
  }

  virtual void recycle() { delete this; }

protected:
  enum State { READY, RUNNING, CANCELING, DONE };

  void reset() {
    _refCount = 1; // this reference is for end user;
    _state = READY;
  }

  virtual size_t doAddRef() { return ++_refCount; }
  virtual size_t doSubRef() { return --_refCount; }

  virtual bool tryTransferState(State from, State to) {
    if (_state == from) {
      _state = to;
      return true;
    }
    return false;
  }

  void setDone() { _state = DONE; }

protected:
  size_t _refCount;
  State _state;
};

class BasicHandle : public HandleBase {
public:
  void startTimer() {
    addRef();
    doStartTimer();
  }

  virtual bool process() {
    if (tryTransferState(READY, RUNNING) && executeTimer()) {
      endTimer();
      return true; // to be removed
    }
    return false;
  }

  virtual bool cancel() override {
    if (tryTransferState(READY, CANCELING) && cancelTimer()) {
      endTimer();
      return true; // to be removed
    }
    return false;
  }

protected:
  virtual void doStartTimer(){}; // eg: notify service
  virtual void doStopTimer(){};  // eg: notify service
  virtual bool executeTimer() {  // return false for asynchrouns execution!
    return true;
  }
  virtual bool cancelTimer() { // return false for asynchrouns cancellation
    return true;
  };
  virtual void afterDone() {} // do callback here
  void endTimer() {
    setDone();
    doStopTimer();
    afterDone();
    subRef();
  }
};

template <class HB> class BasicHandleThreadSafe : public HB {
public:
  virtual size_t doAddRef() override {
    std::lock_guard<std::mutex> lock(_mutex);
    return HB::doAddRef();
  }
  virtual size_t doSubRef() override {
    std::lock_guard<std::mutex> lock(_mutex);
    return HB::doSubRef();
  }

  using State = typename HB::State;
  virtual bool tryTransferState(State from, State to) override {
    std::lock_guard<std::mutex> lock(_mutex);
    return HB::tryTransferState(from, to);
  }

protected:
  std::mutex _mutex;
};

END_ASYNCIO_NAMESPACE;