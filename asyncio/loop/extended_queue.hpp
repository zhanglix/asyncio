#pragma once

#include <asyncio/common.hpp>
#include <deque>
#include <queue>

BEGIN_ASYNCIO_NAMESPACE;
template <class T> class ExtendedQueue : public std::queue<T> {
public:
  template <class V> bool erase(V &&elem) {
    for (auto i = this->c.begin(); i != this->c.end(); i++) {
      if (*i == elem) {
        this->c.erase(i);
        return true;
      }
    }
    return false;
  }
};
END_ASYNCIO_NAMESPACE;