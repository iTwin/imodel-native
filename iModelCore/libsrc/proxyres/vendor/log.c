#include "log.h"

static void (*log_error_func)(const char *fmt, va_list args);
static void (*log_warn_func)(const char *fmt, va_list args);
static void (*log_info_func)(const char *fmt, va_list args);
static void (*log_debug_func)(const char *fmt, va_list args);

void proxy_log_set_error_cb(void (*func)(const char *fmt, va_list args)) {
    log_error_func = func;
}

void proxy_log_set_warn_cb(void (*func)(const char *fmt, va_list args)) {
    log_warn_func = func;
}

void proxy_log_set_info_cb(void (*func)(const char *fmt, va_list args)) {
    log_info_func = func;
}

void proxy_log_set_debug_cb(void (*func)(const char *fmt, va_list args)) {
    log_debug_func = func;
}

void log_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (log_error_func) {
        log_error_func(fmt, args);
    } else {
        vprintf(fmt, args);
        printf("\n");
    }
    va_end(args);
}

void log_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (log_warn_func) {
        log_warn_func(fmt, args);
    } else {
        vprintf(fmt, args);
        printf("\n");
    }
    va_end(args);
}

void log_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (log_info_func)
        log_info_func(fmt, args);
#ifdef _DEBUG
    else {
        vprintf(fmt, args);
        printf("\n");
    }
#endif
    va_end(args);
}

void log_debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (log_debug_func)
        log_debug_func(fmt, args);
#ifdef _DEBUG
    else {
        vprintf(fmt, args);
        printf("\n");
    }
#endif
    va_end(args);
}
