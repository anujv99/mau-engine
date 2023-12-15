#pragma once

#include <cstdio>

#define LOG_COLOR_BLUE "\033[38;5;51m"
#define LOG_COLOR_WHITE "\033[0m"
#define LOG_COLOR_YELLOW "\033[38;5;227m"
#define LOG_COLOR_RED "\033[38;5;196m"
#define LOG_COLOR_PINK "\033[38;5;207m"

#define LOG_WITH_PREFIX(color, prefix, format, ...) printf(color prefix " [%s]: " format "\n" LOG_COLOR_WHITE, MAU_MODULE_NAME, ##__VA_ARGS__);

#define LOG_TRACE(format, ...) LOG_WITH_PREFIX(LOG_COLOR_WHITE, "[T]", format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) LOG_WITH_PREFIX(LOG_COLOR_BLUE, "[I]", format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) LOG_WITH_PREFIX(LOG_COLOR_YELLOW, "[W]", format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) LOG_WITH_PREFIX(LOG_COLOR_RED, "[E]", format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) LOG_WITH_PREFIX(LOG_COLOR_PINK, "[F]", format, ##__VA_ARGS__)
