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

  bool done() const;
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
  virtual bool cancel() final;

protected: // implementing details ...
  bool process();
  void startTimer();
  void endTimer();

  virtual void doStartTimer(); // eg: notify service
  virtual bool executeTimer(); // return false for asynchrouns execution!
  virtual bool cancelTimer();  // return false for asynchrouns cancellation
  virtual void doStopTimer();  // eg: notify service
  virtual void afterDone();    // do callback here
};

template <class HB> class BasicHandleThreadSafe : public HB {
public:
  using HB::HB;
  virtual size_t doAddRef() final {
    std::lock_guard<std::mutex> lock(_mutex);
    return HB::doAddRef();
  }
  virtual size_t doSubRef() final {
    std::lock_guard<std::mutex> lock(_mutex);
    return HB::doSubRef();
  }

  using State = typename HB::State;
  virtual bool tryTransferState(State from, State to) final {
    std::lock_guard<std::mutex> lock(_mutex);
    return HB::tryTransferState(from, to);
  }

protected:
  std::mutex _mutex;
};

END_ASYNCIO_NAMESPACE;