//---------------------------------------------------------------------------------------------
// Copyright (c) Bentley Systems, Incorporated. All rights reserved.
// See COPYRIGHT.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
#pragma once

#include "aecuti.h"

#define USER_NAME_SIZE 32

wchar_t *aecTimeStamp_getUserName( wchar_t * );
int aecTimeStamp_getStamp( SYSTEMTIME * );
