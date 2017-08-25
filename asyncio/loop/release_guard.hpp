#pragma once
#include <asyncio/common.hpp>
#include <functional>

BEGIN_ASYNCIO_NAMESPACE;
template <class T> class ReleaseGuard {
public:
  ReleaseGuard(T *p = nullptr,
               std::function<void(T *p)> releaser = [](T *p) { p->release(); })
      : _p(p), _releaseFunction(releaser) {}
  ReleaseGuard(ReleaseGuard &) = delete;
  ReleaseGuard(ReleaseGuard &&other) {
    T *tmp = other._p;
    other._p = _p;
    _p = tmp;
  }
  T &operator*() const { return *_p; }
  T *operator->() const { return _p; }
  operator T *() const { return _p; }
  ~ReleaseGuard() {
    if (_p) {
      _releaseFunction(_p);
    }
  }

private:
  T *_p;
  std::function<void(T *p)> _releaseFunction;
};
END_ASYNCIO_NAMESPACE;