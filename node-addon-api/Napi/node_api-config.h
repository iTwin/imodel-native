/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#define NODE_ADDON_API_DISABLE_DEPRECATED
#define NAPI_VERSION 5

// the /clr compiler does not support mutex. If we're using managed compile, don't include <mutex>
#if defined (_WIN32) && defined(_MANAGED)
  #define NO_NAPI_MUTEX
#endif
