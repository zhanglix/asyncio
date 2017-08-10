#pragma once

#include <type_traits>

#include <tuple>

#include "../coro/co_gen.hpp"
#include "../coro/coro.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <class... FuturePointers, class A = DefaultAllocator>
coro<void, A> allFutures(FuturePointers &&... args) {
  std::vector<FutureBase *> futs{args...};
  co_await allFutures<A>(futs);
  co_return;
}

template <class A = DefaultAllocator>
coro<void, A> allFutures(std::vector<FutureBase *> &futs) {
  AWaitable<void> awaitable;
  size_t count = futs.size();
  for (auto &&fut : futs) {
    fut->setDoneCallback([&](FutureBase *) {
      if (--count == 0) {
        awaitable.resume();
      }
    });
  }
  co_await awaitable;
  co_return;
}

template <class... CoroArgs>
using FutureTuple =
    std::tuple<Future<typename std::decay_t<CoroArgs>::ReturnType> *...>;

template <class... CoroArgs>
using ResultTuple = std::tuple<typename std::decay_t<CoroArgs>::ReturnType...>;

template <class... CoroArgs>
using CoroTuple = std::tuple<std::decay_t<CoroArgs>...>;

template <class... CoroArgs, class A = DefaultAllocator>
coro<ResultTuple<CoroArgs...>, A> all(EventLoop *loop, CoroArgs &&... args) {
  auto futTuple = std::make_tuple(loop->createTask(args)...);
  std::vector<FutureBase *> futVec = futureTupleToVector(
      futTuple, std::make_index_sequence<sizeof...(CoroArgs)>{});
  co_await allFutures<A>(futVec);
  co_return getFutureTupleResults(
      futTuple, std::make_index_sequence<sizeof...(CoroArgs)>{});
}

template <typename Tuple, std::size_t... I>
auto futureTupleToVector(Tuple &&futures, std::index_sequence<I...>) {
  return std::vector<FutureBase *>{std::get<I>(futures)...};
}

template <typename Tuple, std::size_t... I>
auto getFutureTupleResults(Tuple &&futures, std::index_sequence<I...>) {
  auto ret = std::make_tuple(std::get<I>(futures)->get()...);
  size_t s[] = {std::get<I>(futures)->subRef()...};
  return ret;
}

class FutureCoGen {
public:
  class iterator {
  public:
    iterator(FutureCoGen *af = nullptr) : _anyFuture(af), _next(0) {}
    template <class A = DefaultAllocator> coro<void, A> operator++() {
      if (_anyFuture->hasWaitable(_next)) {
        _fut = co_await _anyFuture->waitable(_next++);
      } else {
        _anyFuture = nullptr;
      }
    }
    bool operator!=(const iterator &other) const {
      return _anyFuture != other._anyFuture;
    }
    bool operator==(const iterator &other) const {
      return _anyFuture == other._anyFuture;
    }
    FutureBase *operator*() { return _fut; }

  protected:
    FutureCoGen *_anyFuture;
    FutureBase *_fut;
    size_t _next;
  };

  FutureCoGen(std::vector<FutureBase *> &futs)
      : _next(0), _futs(futs), _awaitables(_futs.size()) {
    for (auto &&fut : _futs) {
      fut->setDoneCallback(
          [&](FutureBase *h) { _awaitables[_next++].resume(h); });
    }
  }
  ~FutureCoGen() {
    for (auto &&fut : _futs) {
      if (!fut->done()) {
        fut->setDoneCallback([](FutureBase *) {});
      }
    }
  }

  bool hasWaitable(size_t next) { return next < _futs.size(); }
  AWaitable<FutureBase *> &waitable(size_t pos) { return _awaitables[pos]; }
  FutureBase *future(size_t pos) { return _futs[pos]; }
  template <class A = DefaultAllocator> coro<iterator, A> begin() {
    auto iter = iterator(this);
    co_await++ iter;
    co_return iter;
  }

  iterator end() const { return iterator(nullptr); }

protected:
  std::vector<FutureBase *> _futs;
  std::vector<AWaitable<FutureBase *>> _awaitables;
  size_t _next;
};

END_ASYNCIO_NAMESPACE;