#pragma once

#include <stdarg.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void proxy_log_set_error_cb(void (*func)(const char *fmt, va_list args));
void proxy_log_set_warn_cb(void (*func)(const char *fmt, va_list args));
void proxy_log_set_info_cb(void (*func)(const char *fmt, va_list args));
void proxy_log_set_debug_cb(void (*func)(const char *fmt, va_list args));

void log_error(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_debug(const char *fmt, ...);

#ifdef __cplusplus
}
#endif
