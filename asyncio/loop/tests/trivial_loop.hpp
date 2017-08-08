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
  void push(TrivialTimerHandle *h);
  virtual bool cancelTimer(TrivialTimerHandle *handle);
  virtual void recycleTimerHandle(TrivialTimerHandle *handle) { delete handle; }

  class TrivialTimerHandle : public TimerHandle {
  public:
    TrivialTimerHandle(TrivialLoop *lc, TimerCallback cb, void *data)
        : TimerHandle(data), _cb(cb), _lc(lc) {}

    virtual void recycle() override { _lc->recycleTimerHandle(this); }

    void setupTimer() { startTimer(); }
    void processEntry() { process(); }
    void doStartTimer() override { _lc->push(this); }
    bool cancelTimer() override { return _lc->cancelTimer(this); }
    bool executeTimer() override {
      (*_cb)(this);
      return true;
    }

  protected:
    TimerCallback _cb;
    TrivialLoop *_lc;
  };

private:
  std::vector<TrivialTimerHandle *> _timers;
  uint64_t _now;
};
END_ASYNCIO_NAMESPACE;