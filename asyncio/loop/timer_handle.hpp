#pragma once

#include <asyncio/common.hpp>

#include "handle_base.hpp"

BEGIN_ASYNCIO_NAMESPACE;
class TimerHandle : public BasicHandle {
public:
  TimerHandle(void *data = nullptr);
  void *data() const;
  void setData(void *data);
  void reset(void *data = nullptr);

protected:
  void *_data;
};

END_ASYNCIO_NAMESPACE;
