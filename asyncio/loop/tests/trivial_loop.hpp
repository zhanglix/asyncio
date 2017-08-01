#pragma once
#include <vector>

#include "../loop_core.hpp"
#include "../timer_handle.hpp"
#include <asyncio/common.hpp>

BEGIN_ASYNCIO_NAMESPACE;

class TrivialLoop : public LoopCore {
public:
  TrivialLoop() : _now(0) {}
  virtual ~TrivialLoop() {}
  virtual void runOneIteration() override;
  virtual void close() override;
  virtual size_t activeHandlesCount() override;
  virtual uint64_t time() override;
  virtual TimerHandle *callSoon(TimerCallback callback, void *data) override;
  virtual TimerHandle *callSoonThreadSafe(TimerCallback callback,
                                          void *data) override;
  virtual TimerHandle *callLater(uint64_t milliseconds, TimerCallback callback,
                                 void *data) override;

  class TrivialTimerHandle;
  virtual bool cancelTimer(TrivialTimerHandle *handle);
  virtual void recycleTimerHandle(TrivialTimerHandle *handle) { delete handle; }

  // DefautTimerHandle To be deleted!
  class TrivialTimerHandle : public TimerHandle {
  public:
    enum State { READY = 0, FINISHED, CANCELED };

    TrivialTimerHandle(TrivialLoop *lc, void *data)
        : TimerHandle(data), _loop(lc), _state(READY) {}
    TrivialTimerHandle(TrivialLoop *lc) : TrivialTimerHandle(lc, nullptr) {}

    virtual LoopCore *loopCore() const override { return _loop; }

    virtual void release() override { _loop->recycleTimerHandle(this); }

    bool cancel() override { return _loop->cancelTimer(this); }
    bool completed() override { return _state != State::READY; }

    State getState() const { return _state; }
    void setState(TrivialTimerHandle::State state) { _state = state; }

    void reset(void *data = nullptr) override {
      TimerHandle::reset(data);
      _state = State::READY;
    }

    TimerCallback callback;

    static void subRefOnLoop(TimerHandle *handle);

  protected:
    State _state;
    TrivialLoop *_loop;
  };

private:
  std::vector<TrivialTimerHandle *> _timers;
  uint64_t _now;
};
END_ASYNCIO_NAMESPACE;