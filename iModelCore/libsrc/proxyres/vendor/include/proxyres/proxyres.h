#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "config.h"
#include "resolver.h"
#include "execute.h"
#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize proxy resolution library
bool proxyres_global_init(void);

// Uninitialize proxy resolution library
bool proxyres_global_cleanup(void);

#ifdef __cplusplus
}
#endif
