//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#pragma once

#include "aecuti.h"

#define USER_NAME_SIZE 32

wchar_t *aecTimeStamp_getUserName( wchar_t * );
int aecTimeStamp_getStamp( SYSTEMTIME * );
