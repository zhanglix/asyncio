#pragma once

#ifdef ENABLE_ASYNCIO_LOG
#include <spdlog/spdlog.h>

static auto _async_io_logger_ = spdlog::get("asyncio")
                                    ? spdlog::get("asyncio")
                                    : spdlog::stderr_logger_mt("asyncio");

#define LOG_DEBUG(fmt, ...) _async_io_logger_->debug(fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) _async_io_logger_->info(fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) _async_io_logger_->warn(fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) _async_io_logger_->error(fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)
#define LOG_INFO(fmt, ...)
#define LOG_WARN(fmt, ...)
#define LOG_ERROR(fmt, ...)
#endif