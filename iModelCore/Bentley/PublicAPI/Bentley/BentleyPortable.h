/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

// This header file is for the use of client code that uses Bentley.dll

#ifdef BENTLEY_H
    #error Include this file first, before any other Bentley or DgnPlatform header file, if you want to make your code able to include Bentley portability headers.
#endif

#include "Bentley.h"

#define BENTLEY_PORTABLE_H_

