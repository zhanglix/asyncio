#pragma once
#include <asyncio/common.hpp>
#include <cstddef>
#include <mutex>
#include <utility>

BEGIN_ASYNCIO_NAMESPACE;

class HandleBase {
public:
  HandleBase();
  virtual ~HandleBase();

  virtual bool done() const;
  virtual bool cancel() = 0;

  size_t refCount() const;
  size_t addRef();
  size_t subRef();

protected:
  virtual void recycle();
  enum State { READY, RUNNING, CANCELING, DONE };
  void reset();
  virtual size_t doAddRef();
  virtual size_t doSubRef();
  virtual bool tryTransferState(State from, State to);
  void setDone();

protected:
  size_t _refCount;
  State _state;
};

class BasicHandle : public HandleBase {
public: // DON'T add any public method here
  virtual bool cancel() override;

protected: // implementing details ...
  virtual bool process();
  void startTimer();
  void endTimer();

  virtual void doStartTimer(); // eg: notify service
  virtual void doStopTimer();  // eg: notify service
  virtual bool executeTimer(); // return false for asynchrouns execution!
  virtual bool cancelTimer();  // return false for asynchrouns cancellation
  virtual void afterDone();    // do callback here
};

template <class HB> class BasicHandleThreadSafe : public HB {
public:
  template <class... Args>
  BasicHandleThreadSafe(Args &&... args) : HB(std::forward<Args>(args)...) {}

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