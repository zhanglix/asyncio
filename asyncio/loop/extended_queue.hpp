#pragma once

#include <asyncio/common.hpp>
#include <deque>
#include <exception>
#include <queue>
#include <unordered_set>

BEGIN_ASYNCIO_NAMESPACE;
template <class T> class ExtendedQueue : public std::queue<T> {
public: // optimized me when needed
  template <class V> bool erase(V &&elem) {
    if (_set.erase(elem) == 0) {
      return false;
    }
    for (auto i = this->c.begin(); i != this->c.end(); i++) {
      if (*i == elem) {
        this->c.erase(i);
        break;
      }
    }
    return true;
  }

  template <class V> void push(V &&v) {
    auto pair = _set.insert(v);
    if (pair.second) {
      std::queue<T>::push(v);
    } else {
      throw std::logic_error("duplicate elements detected!");
    }
  }

  void pop() {
    _set.erase(this->front());
    std::queue<T>::pop();
  }

private:
  std::unordered_set<T> _set;
};
END_ASYNCIO_NAMESPACE;