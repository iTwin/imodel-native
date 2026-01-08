#pragma once

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void proxy_log_set_error_cb(void (*func)(const char *fmt, va_list args));

void proxy_log_set_warn_cb(void (*func)(const char *fmt, va_list args));

void proxy_log_set_info_cb(void (*func)(const char *fmt, va_list args));

void proxy_log_set_debug_cb(void (*func)(const char *fmt, va_list args));

#ifdef __cplusplus
}
#endif
